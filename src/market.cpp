#include <nlohmann/json.hpp>
#include <string>
#include <iostream>
#include <map>
#include "market.h"

using json = nlohmann::json;

Market::Market() {
    Market::setQuestion("");
    Market::setTokenId("");
    Market::setBid(0);
    Market::setAsk(0);
}

Market::Market(json j) {
    if (j["question"].is_null()) { Market::setQuestion(""); }
    else { Market::setQuestion(j["question"].get<std::string>()); }

    if (j["clobTokenIds"].is_null()) { Market::setTokenId(""); }
    else {
        std::string rawIds = j["clobTokenIds"].get<std::string>();
        json ids = json::parse(rawIds);
        Market::setTokenId(ids[0].get<std::string>());
    }

    if (j["bestBid"].is_null()) { Market::setBid(0); }
    else { Market::setBid(j["bestBid"].get<double>()); }

    if (j["bestAsk"].is_null()) { Market::setAsk(0); }
    else { Market::setAsk(j["bestAsk"].get<double>()); }
}

std::string Market::getQuestion() { return question_; }
std::string Market::getTokenId() { return tokenId_; }
double Market::getBid() { return bid_; }
double Market::getAsk() { return ask_; }
std::map<double, double, std::greater<double>> Market::getBids() { return bids_; }
std::map<double, double> Market::getAsks() { return asks_; }

void Market::setQuestion(std::string q) { question_ = q; }
void Market::setTokenId(std::string id) { tokenId_ = id; }
void Market::setBid(double b) { bid_ = b; }
void Market::setAsk(double a) { ask_ = a; }
void Market::setBids(json j) {
    for (auto& bid : j) {
        double price = std::stod(bid["price"].get<std::string>()); 
        double size = std::stod(bid["size"].get<std::string>());
        bids_[price] = size;
    }
}
void Market::setAsks(json j) {
    for (auto& ask : j) {
        double size = std::stod(ask["size"].get<std::string>());
        double price = std::stod(ask["price"].get<std::string>());
        asks_[price] = size;
    }
}

void Market::updateOrders(json j) {
    double size = std::stod(j["size"].get<std::string>());
    double price = std::stod(j["price"].get<std::string>());
    std::string orderType = j["side"].get<std::string>();

        if (orderType == "BUY") {
            if (size == 0) { bids_.erase(price); }
            else { bids_[price] = size; }
        } else {
            if (size == 0) { asks_.erase(price); }
            else { asks_[price] = size; }
        }
    // std::cout << orderType + " " << size << " at $" << price << "\n";

}

std::ostream& operator<<(std::ostream& os, const Market& m) {
    double chance = (m.ask_ + m.bid_) / 2 * 100;
    if (m.bid_ == 0 && (m.ask_ <= 0.01 || m.ask_ == 1)) { chance = 0; }
    os << "  -" << m.question_ << "\n";
    os << "     -Bid:    $" << m.bid_ << "\n";
    os << "     -Ask:    $" << m.ask_ << "\n";
    os << "     -Chance:  " << chance << "%\n";
    
    return os;
}