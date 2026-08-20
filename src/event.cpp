#include <nlohmann/json.hpp>
#include <string>
#include <iostream>
#include <vector>
#include "event.h"
#include "market.h"

Event::Event() { setTitle(""); }

Event::Event(nlohmann::json j) {
    if (j["title"].is_null()) { Event::setTitle(""); }
    else { Event::setTitle(j["title"].get<std::string>()); }
    
    if (!j["markets"].is_null()) {
        for (auto& market : j["markets"]) {
            Event::addMarket(new Market(market));
        }
    }
}

Event::Event(std::string t) { Event::setTitle(t); }

Event::Event(std::string t, std::vector<Market*> markets) {
    Event::setTitle(t);
    Event::setMarkets(markets);
}

Event::~Event() {
    for(Market* m : markets) {
        delete m;
    }
}

std::string Event::getTitle() { return title; }
std::vector<Market*> Event::getMarkets() { return markets; }

void Event::setTitle(std::string t) { title = t; }
void Event::setMarkets(std::vector<Market*> markets) {
    for(auto& m : markets) {
        markets.push_back(m);
    }
}
void Event::addMarket(Market* m) {
    markets.push_back(m);
}

std::ostream& operator<<(std::ostream& os, const Event& e) {
    os << e.title << "\n";
    for (const Market* m : e.markets) {
        os << *m;
    }

    return os;
}