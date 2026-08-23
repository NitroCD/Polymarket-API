#include <nlohmann/json.hpp>
#include <string>
#include <iostream>
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

void Market::setQuestion(std::string q) { question_ = q; }
void Market::setTokenId(std::string id) { tokenId_ = id; }
void Market::setBid(double b) { bid_ = b; }
void Market::setAsk(double a) { ask_ = a; }

std::ostream& operator<<(std::ostream& os, const Market& m) {
    double chance = (m.ask_ + m.bid_) / 2 * 100;
    if (m.bid_ == 0 && (m.ask_ <= 0.01 || m.ask_ == 1)) { chance = 0; }
    os << "  -" << m.question_ << "\n";
    os << "     -Bid:    $" << m.bid_ << "\n";
    os << "     -Ask:    $" << m.ask_ << "\n";
    os << "     -Chance:  " << chance << "%\n";
    
    return os;
}