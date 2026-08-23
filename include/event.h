#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <iostream>
#include <vector>
#include "market.h"

class Event {
    private:
        std::string title_;
        std::vector<Market*> markets_;

    public:
        Event();
        Event(nlohmann::json);

        ~Event();

        std::string getTitle();
        std::vector<Market*> getMarkets();

        void setTitle(std::string);
        void setMarkets(std::vector<Market*>);
        void addMarket(Market*);

        friend std::ostream& operator<<(std::ostream&, const Event&);
};