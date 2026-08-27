#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <iostream>
#include <map>
#include <deque>

class Market {
    private:
        std::string question_;
        std::string tokenId_;
        double bid_;
        double ask_;
        std::map<double, double, std::greater<double>> bids_;
        std::map<double, double> asks_;
        std::deque<std::string> recentTrades;

    public:
        Market();
        Market(nlohmann::json);

        ~Market() {};

        std::string getQuestion();
        std::string getTokenId();
        double getBid();
        double getAsk();
        std::map<double, double, std::greater<double>> getBids();
        std::map<double, double> getAsks();

        void setQuestion(std::string);
        void setTokenId(std::string);
        void setBid(double);
        void setAsk(double);
        void setBids(nlohmann::json);
        void setAsks(nlohmann::json);
        void updateOrders(nlohmann::json);

        friend std::ostream& operator<<(std::ostream&, const Market&);
};

