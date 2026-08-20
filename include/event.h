#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <iostream>
#include <vector>
#include "market.h"

class Event {
    private:
        std::string title;
        std::vector<Market*> markets;

    public:
        Event();
        Event(nlohmann::json);
        Event(std::string);
        Event(std::string, std::vector<Market*>);

        ~Event();

        std::string getTitle();
        std::vector<Market*> getMarkets();

        void setTitle(std::string);
        void setMarkets(std::vector<Market*>);
        void addMarket(Market*);

        friend std::ostream& operator<<(std::ostream&, const Event&);
};