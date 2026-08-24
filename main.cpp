#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <iomanip>
#include <string>
#include <thread>
#include <chrono>
#include "event.h"
#include "market.h"

using json = nlohmann::json;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* out) {
    size_t totalSize = size * nmemb;
    out->append((char*)contents, totalSize);
    return totalSize;
}

std::string getResponse(CURL* curl, const std::string& url) {
    std::string response;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        throw std::runtime_error(curl_easy_strerror(res));
    }
    curl_easy_cleanup(curl);

    return response;
}

void initOrders(Market* m, json j) {
    json bids = j["bids"];
    json asks = j["asks"];
    m->setBids(bids);
    m->setAsks(asks);
}

void getLivePrice(Market* m) {
    std::string tokenId = m->getTokenId();
    CURL* wsHandle = curl_easy_init();
    curl_easy_setopt(wsHandle, CURLOPT_URL, "wss://ws-subscriptions-clob.polymarket.com/ws/market");
    curl_easy_setopt(wsHandle, CURLOPT_CONNECT_ONLY, 2L);

    CURLcode res = curl_easy_perform(wsHandle);
    if (res != CURLE_OK) {
        throw std::runtime_error(curl_easy_strerror(res));
    }

    std::string subscribeMsg = "{\"assets_ids\": [\"" + tokenId + "\"], \"type\": \"market\"}";

    size_t sent;
    curl_ws_send(wsHandle, subscribeMsg.c_str(), subscribeMsg.size(), &sent, 0, CURLWS_TEXT);

    bool hasInit = false;

    // Read messages in a loop
    char buffer[65536];
    while (true) {
        size_t recvLen;
        const struct curl_ws_frame* meta;
        CURLcode readRes = curl_ws_recv(wsHandle, buffer, sizeof(buffer), &recvLen, &meta);

        if (readRes == CURLE_OK) {
            std::string response(buffer, recvLen);
            json j = json::parse(response);
            std::cout << "Received: ";
            if (j.is_array()) { j = j[0]; }
            if (j.contains("price_changes")) {
                for (auto& change : j["price_changes"]) {
                    if (change["asset_id"].get<std::string>() != m->getTokenId()) { continue; }
                    m->updateOrders(change);
                }
                continue;  // or restructure the branch
            }
            if (!hasInit) {
                initOrders(m, j);
                hasInit = true;
                std::cout << "initial prices" << "\n";
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        } else if (readRes == CURLE_AGAIN) {
            continue; // no data yet, just poll again
        } else {
            std::cerr << "Read error: " << curl_easy_strerror(readRes) << "\n";
            break;
        }
    }
    curl_easy_cleanup(wsHandle);
}

int main () {
    std::cout << std::fixed << std::setprecision(3);

    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to init curl\n";
        return 1;
    }

    std::string response;
    // std::string url = "https://gamma-api.polymarket.com/events?limit=5&active=true&closed=false";
    std::string url = "https://gamma-api.polymarket.com/events?slug=2026-f1-drivers-champion";

    try {
        response = getResponse(curl, url);
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }

    curl_easy_cleanup(curl);

    json data = json::parse(response);

    std::vector<std::unique_ptr<Event>> events;
    for (auto& event : data) {
        events.push_back(std::make_unique<Event>(event));
    }

    for (const std::unique_ptr<Event>& e : events) {
        std::cout << *e << "\n";
    }

    std::vector<Market*> markets = events[0]->getMarkets();

    try {
        getLivePrice(markets[3]);
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
    
    return 0;
}