#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <iomanip>
#include <string>

using json = nlohmann::json;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* out) {
    size_t totalSize = size * nmemb;
    out->append((char*)contents, totalSize);
    return totalSize;
}

int main () {
    std::cout << std::fixed << std::setprecision(2);

    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to init curl\n";
        return 1;
    }

    std::string response;
    std::string url = "https://gamma-api.polymarket.com/events?limit=5&active=true&closed=false";

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "curl error: " << curl_easy_strerror(res) << "\n";
        curl_easy_cleanup(curl);
        return 1;
    }
    curl_easy_cleanup(curl);

    json data = json::parse(response);
    for (auto& event : data) {
        std::cout << event["title"] << "\n";
        for (auto& market : event["markets"]) {
            bool nullBid = market["bestBid"].is_null();
            bool nullAsk = market["bestAsk"].is_null();
            if (nullBid || nullAsk) { continue; }
            double bid, ask, chance;
            if (!nullBid) { bid = market["bestBid"].get<double>(); }
            if (!nullAsk) { ask = market["bestAsk"].get<double>(); }
            if (nullAsk) {
                chance = 1;
            } else if (nullBid) {
                chance = 0;
            } else {
                chance = (ask + bid) / 2;
            }
            chance *= 100;
            std::cout << "  -" << market["question"] << "\n";
            std::cout << "      -Bid: " << market["bestBid"] << "\n";
            std::cout << "      -Ask: " << market["bestAsk"] << "\n";
            std::cout << "      -Chance: " << chance << "%\n";
        }
    }

    return 0;
}