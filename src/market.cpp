#include <nlohmann/json.hpp>
#include <format>
#include <string>
#include <iostream>
#include <map>
#include "market.h"

using json = nlohmann::json;

Market::Market() {
    Market::setQuestion("");
    Market::setTokenId("");
    Market::setLivePrice(0);
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

    if (j["lastTradePrice"].is_null()) { Market::setLivePrice(0); }
    else { Market::setLivePrice(j["lastTradePrice"].get<double>()); }
}

std::string Market::getQuestion() { return question_; }
std::string Market::getTokenId() { return tokenId_; }
double Market::getLivePrice() { return livePrice_; }
double Market::getBestBid() { return bestBid_; }
double Market::getBestAsk() { return bestAsk_; }
std::map<double, double, std::greater<double>> Market::getBids() { return bids_; }
std::map<double, double> Market::getAsks() { return asks_; }
std::deque<std::string> Market::getRecentTrades() { return recentTrades_; }

void Market::setQuestion(std::string q) { question_ = q; }
void Market::setTokenId(std::string id) { tokenId_ = id; }
void Market::setLivePrice(double p) { livePrice_ = p; }
void Market::setBestBid(double b) { bestBid_ = b; }
void Market::setBestAsk(double a) { bestAsk_ = a; }
void Market::setBids(json j) {
    bids_.clear();
    for (auto& bid : j) {
        double price = std::stod(bid["price"].get<std::string>()); 
        double size = std::stod(bid["size"].get<std::string>());
        bids_[price] = size;
    }
}
void Market::setAsks(json j) {
    asks_.clear();
    for (auto& ask : j) {
        double size = std::stod(ask["size"].get<std::string>());
        double price = std::stod(ask["price"].get<std::string>());
        asks_[price] = size;
    }
}

void Market::clearRecentTrades() { recentTrades_.clear(); }

void Market::updateOrders(json j) {
    double size = std::stod(j["size"].get<std::string>());
    double price = std::stod(j["price"].get<std::string>());
    std::string orderType = j["side"].get<std::string>();

    if (orderType == "BUY") {
        double oldBidSize;
        if (bids_.contains(price)) {
            oldBidSize = bids_[price];
        } else { oldBidSize = 0; }

        if (size == 0) {
            bids_.erase(price);
        }
        else {
            bids_[price] = size;
        }
        updateTrades(size - oldBidSize, price, true);
    } else {
        double oldAskSize;
        if (asks_.contains(price)) {
            oldAskSize = asks_[price];
        } else { oldAskSize = 0; }

        if (size == 0) {
            asks_.erase(price);
        }
        else {
            asks_[price] = size;
        }
        updateTrades(size - oldAskSize, price, false);
    }
}

void Market::updateTrades(const double& size, const double& price, bool isBid) {
    std::string orderType;
    std::string orderAction;
    std::string suffix = std::string(std::format("{:.2f}", abs(size)) + " at $" + std::format("{:.3f}", price));

    if (size >= 0) {
        orderType = "LMT ";
    } else {
        orderType = "MKT ";
    }

    if ((isBid && size > 0) || (!isBid && size < 0)) {
        orderAction = "BUY ";
    } else {
        orderAction = "SELL ";
    }

    recentTrades_.push_back(std::string(orderType + orderAction + suffix));

    if (recentTrades_.size() > 30) { recentTrades_.pop_front(); }
}

std::ostream& operator<<(std::ostream& os, const Market& m) {
    os << "  -" << m.question_ << "\n";
    return os;
}