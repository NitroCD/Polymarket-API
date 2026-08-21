#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <iomanip>
#include <string>
#include "event.h"
#include "market.h"

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
    //std::string url = "https://gamma-api.polymarket.com/events?limit=5&active=true&closed=false";
    std::string url = "https://gamma-api.polymarket.com/events?slug=2026-f1-drivers-champion";

    try {
        response = getResponse(curl, url);
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }

    json data = json::parse(response);

    std::vector<std::unique_ptr<Event>> events;
    for (auto& event : data) {
        events.push_back(std::make_unique<Event>(event));
    }

    for (const std::unique_ptr<Event>& e : events) {
        std::cout << *e << "\n";
    }

    curl_easy_cleanup(curl);
    return 0;
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