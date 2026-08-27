#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <deque>
#include <thread>
#include <chrono>
#include <ctime>
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

CURL* connectAndSubscribe(const std::string& tokenId) {
    CURL* wsHandle = curl_easy_init();
    curl_easy_setopt(wsHandle, CURLOPT_URL, "wss://ws-subscriptions-clob.polymarket.com/ws/market");
    curl_easy_setopt(wsHandle, CURLOPT_CONNECT_ONLY, 2L);

    CURLcode res = curl_easy_perform(wsHandle);
    if (res != CURLE_OK) {
        throw std::runtime_error(curl_easy_strerror(res));
    }

    std::string subscribeMsg = "{\"assets_ids\": [\"" + tokenId + "\"], \"type\": \"market\", \"custom_feature_enabled\": true}";

    size_t sent;
    curl_ws_send(wsHandle, subscribeMsg.c_str(), subscribeMsg.size(), &sent, 0, CURLWS_TEXT);

    return wsHandle;
}

CURL* reconnectWithDeadline(const std::string& tokenId, time_t windowEnd) {
    CURL* curl = connectAndSubscribe(tokenId);
    while (!curl && time(nullptr) < windowEnd) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        curl = connectAndSubscribe(tokenId);
    }
    return curl;  // either a valid handle, or nullptr if we ran out of time
}

void initOrders(Market* m, json j) {
    // std::cout << "before initOrders" << std::endl;
    json bids = j["bids"];
    json asks = j["asks"];
    m->setBids(bids);
    m->setAsks(asks);
    m->clearRecentTrades();

    // std::cout << "after initOrders" << std::endl;
}

std::string formatRow(std::map<double, double>::const_iterator it, 
                       std::map<double, double>::const_iterator end) {
    std::ostringstream row;
    if (it != end) {
        row << "$" << std::setw(6) << it->first << ": " << std::setw(10) << it->second;
    } else {
        row << std::string(19, ' ');
    }
    return row.str();
}

void createOrderBook(Market& m) {
    std::ostringstream frame;
    frame << std::fixed << std::setprecision(2);
    frame << "\033[2J\033[1;1H";

    frame << "BIDS" << std::setw(39) << "ASKS" << std::setw(41) << "ORDERS";
    frame << std::setw(39) << "LIVE PRICE: $" << m.getLivePrice() << "\n";

    frame << std::fixed << std::setprecision(3);

    const std::map<double, double, std::greater<double>>& bids = m.getBids();
    const std::map<double, double>& asks = m.getAsks();
    std::deque<std::string> recentTrades = m.getRecentTrades();
    auto bidit = bids.begin();
    auto askit = asks.begin();
    auto tradesit = recentTrades.begin();

    for (int i = 0; i < 30; i++) {
        frame << formatRow(bidit, bids.end());
        frame << std::setw(20) << " ";
        frame << formatRow(askit, asks.end());
        frame << std::setw(20) << " ";
        frame << std::left;
        if (tradesit != recentTrades.end()) {
            frame << std::setw(35) << *tradesit;
        } else {
            frame << std::setw(35) << " ";
        }
        frame << std::right;
        frame << "\n";

        if (bidit != bids.end()) ++bidit;
        if (askit != asks.end()) ++askit;
        if (tradesit != recentTrades.end()) ++tradesit;
    }
    std::cout << frame.str();
}

void getLivePrice(Market* m) {
    std::string tokenId = m->getTokenId();
    CURL* curl = connectAndSubscribe(tokenId);
    std::cout << "Initial connect result: " << (curl ? "success" : "FAILED") << "\n";

    bool hasInit = false;
    std::string fullMessage;

    time_t now = time(nullptr);
    time_t windowStart = now - (now % 300);
    time_t windowEnd = windowStart + 300;

    // Read messages in a loop
    char buffer[65536];
    while (true) {
        if (time(nullptr) >= windowEnd) {
            std::cout << "Exiting: window ended\n";
            break;
        }

        size_t recvLen;
        const struct curl_ws_frame* meta;
        CURLcode readRes = curl_ws_recv(curl, buffer, sizeof(buffer), &recvLen, &meta);

        if (readRes == CURLE_OK) {
            if (meta->flags & CURLWS_CLOSE) {
                std::cerr << "Server sent CLOSE frame, reconnecting...\n";
                curl_easy_cleanup(curl);
                curl = connectAndSubscribe(tokenId);
                while (!curl) {
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    curl = connectAndSubscribe(tokenId);
                }
                fullMessage.clear();
                hasInit = false;
                continue;
            }
            fullMessage.append(buffer, recvLen);

            if (meta->bytesleft == 0) {
                json j = json::parse(fullMessage);
                fullMessage.clear();
                if (j.is_array()) { j = j[0]; }
                if (j.contains("price_changes")) {
                    for (auto& change : j["price_changes"]) {
                        if (change["asset_id"].get<std::string>() != m->getTokenId()) { continue; }
                        m->updateOrders(change);
                        createOrderBook(*m);
                    }
                    continue;  // or restructure the branch
                } else if (j.contains("event_type")) {
                   if (j["event_type"].get<std::string>() == "last_trade_price"
                        && j["asset_id"].get<std::string>() == m->getTokenId()) {
                        m->setLivePrice(stod(j["price"].get<std::string>()));
                   }
                   if (j["event_type"].get<std::string>() == "best_bid_ask"
                        && j["asset_id"].get<std::string>() == m->getTokenId()) {
                        // update best bid/ask on market
                   }
                }
                if (!hasInit) {
                    initOrders(m, j);
                    hasInit = true;
                    createOrderBook(*m);
                }
            }
        } else if (readRes == CURLE_AGAIN) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        } else {
            std::cout << "curl_ws_recv failed: " << curl_easy_strerror(readRes) << "\n";
            curl_easy_cleanup(curl);
            curl = reconnectWithDeadline(tokenId, windowEnd);
            if (!curl) {
                std::cout << "Exiting: reconnect failed before window ended\n";
                break;
            }

            fullMessage.clear();
            hasInit = false;
            continue;
        }
    }
    curl_easy_cleanup(curl);
}

std::string getCurrentURL() {
    time_t now = time(nullptr);
    time_t windowStart = now - (now % 300);
    std::string slug = "btc-updown-5m-" + std::to_string(windowStart);
    std::string url = "https://gamma-api.polymarket.com/events?slug=" + slug;
    return url;
}

std::vector<std::unique_ptr<Event>> getCurrentEvents(CURL* curl, std::string url) {
    std::string response = getResponse(curl, url);
    curl_easy_cleanup(curl);

    json data = json::parse(response);

    std::vector<std::unique_ptr<Event>> events;
    for (auto& event : data) {
        events.push_back(std::make_unique<Event>(event));
    }
    return events;
}

int main () {
    while(true) {
        CURL* curl = curl_easy_init();
        if (!curl) {
            std::cerr << "Failed to init curl\n";
            return 1;
        }
        std::string url = getCurrentURL();
        std::cout << "URL: " << url << "\n";

        std::vector<std::unique_ptr<Event>> events;

        try {
            events = getCurrentEvents(curl, url);
        } catch (const std::exception& e) {
            std::cerr << e.what() << "\n";
            return 1;
        }

        if (events.empty()) {
            std::cerr << "No events returned, skipping this cycle \n";
            continue;
        }
        
        std::vector<Market*> markets = events[0]->getMarkets();

        try {
            if (!markets.empty() && markets[0] != nullptr) {
                getLivePrice(markets[0]);
            } else {
                std::cerr << "No markets found, skipping this cycle \n";
            }
        } catch (const std::exception& e) {
            std::cerr << e.what() << "\n";
            return 1;
        }
    }
    return 0;
}