#include <nlohmann/json.hpp>
#include <string>
#include <iostream>
#include <vector>
#include "event.h"
#include "market.h"

Event::Event() { setTitle(""); }

Event::Event(nlohmann::json j) {
    if (j["title"].is_null()) {
        Event::setTitle("");
    } else {
        Event::setTitle(j["title"].get<std::string>());
    }
    
    if (!j["markets"].is_null()) {
        for (auto& market : j["markets"]) {
            Event::addMarket(new Market(market));
        }
    }
}

Event::~Event() {
    for(Market* m : markets_) {
        delete m;
    }
}

std::string Event::getTitle() { return title_; }
std::vector<Market*> Event::getMarkets() { return markets_; }

void Event::setTitle(std::string t) { title_ = t; }
void Event::setMarkets(std::vector<Market*> markets) {
    for(auto& m : markets) {
        markets_.push_back(m);
    }
}
void Event::addMarket(Market* m) {
    markets_.push_back(m);
}

std::ostream& operator<<(std::ostream& os, const Event& e) {
    os << e.title_ << "\n";
    for (const Market* m : e.markets_) {
        os << *m;
    }

    return os;
}