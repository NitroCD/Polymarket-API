#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

using json = nlohmann::json;

int main() {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to init curl\n";
        return 1;
    }

    std::string url = "wss://ws-subscriptions-clob.polymarket.com/ws/market";
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_CONNECT_ONLY, 2L);  // 2L = WebSocket mode

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "Connect failed: " << curl_easy_strerror(res) << "\n";
        curl_easy_cleanup(curl);
        return 1;
    }

    std::cout << "Connected!\n";

    /*
    // Send the subscription message
    std::string rawIds = "[\"36135303630970774358991758965953725374791089628290212294816140371870983436829\", \"100818499003359884559774352671825765184103958625070066814408985574716007147193\"]";
    json ids = json::parse(rawIds);        // parse the string into a real JSON array
    std::string tokenId = ids[0].get<std::string>();  // pull out just ONE token id
    */
    
    std::string tokenId = "32950178421556833525068948927823594772134813180063196823389171317494746105102";

    std::string subscribeMsg = "{\"assets_ids\": [\"" + tokenId + "\"], \"type\": \"market\"}";

    size_t sent;
    curl_ws_send(curl, subscribeMsg.c_str(), subscribeMsg.size(), &sent, 0, CURLWS_TEXT);

    // Read messages in a loop
    char buffer[65536];
    while (true) {
        size_t recvLen;
        const struct curl_ws_frame* meta;
        CURLcode readRes = curl_ws_recv(curl, buffer, sizeof(buffer), &recvLen, &meta);

        if (readRes == CURLE_OK) {
            std::cout << "Received: " << std::string(buffer, recvLen) << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        } else if (readRes == CURLE_AGAIN) {
            continue; // no data yet, just poll again
        } else {
            std::cerr << "Read error: " << curl_easy_strerror(readRes) << "\n";
            break;
        }
    }

    curl_easy_cleanup(curl);
    return 0;
}