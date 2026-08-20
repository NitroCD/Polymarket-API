#include <nlohmann/json.hpp>
#include <string>
#include <iostream>
#include "market.h"

using json = nlohmann::json;

Market::Market() {
    question = "";
    ask = 0;
    bid = 1;
}

Market::Market(std::string q, double b, double a) {
    Market::setQuestion(q);
    Market::setBid(b);
    Market::setAsk(a);
}

Market::Market(json j) {
    if (j["question"].is_null()) { Market::setQuestion(""); }
    else { Market::setQuestion(j["question"].get<std::string>()); }

    if (j["bestBid"].is_null()) { Market::setBid(0); }
    else { Market::setBid(j["bestBid"].get<double>()); }

    if (j["bestAsk"].is_null()) { Market::setAsk(0); }
    else { Market::setAsk(j["bestAsk"].get<double>()); }
}

std::string Market::getQuestion() { return question; }
double Market::getBid() { return bid; }
double Market::getAsk() { return ask; }

void Market::setQuestion(std::string q) { question = q; }
void Market::setBid(double b) { bid = b; }
void Market::setAsk(double a) { ask = a; }

std::ostream& operator<<(std::ostream& os, const Market& m) {
    const double chance = (m.ask + m.bid) / 2 * 100;
    os << "  -" << m.question << "\n";
    os << "     -Bid:    $" << m.bid << "\n";
    os << "     -Ask:    $" << m.ask << "\n";
    os << "     -Chance:  " << chance << "%\n";
    
    return os;
}