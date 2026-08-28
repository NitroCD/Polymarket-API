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
        double livePrice_;
        double bestBid_;
        double bestAsk_;
        std::map<double, double, std::greater<double>> bids_;
        std::map<double, double> asks_;
        std::deque<std::string> recentTrades_;

    public:
        Market();
        Market(nlohmann::json);

        ~Market() {};

        std::string getQuestion();
        std::string getTokenId();
        double getLivePrice();
        double getBestBid();
        double getBestAsk();
        std::map<double, double, std::greater<double>> getBids();
        std::map<double, double> getAsks();
        std::deque<std::string> getRecentTrades();

        void setQuestion(std::string);
        void setTokenId(std::string);
        void setLivePrice(double);
        void setBestBid(double);
        void setBestAsk(double);
        void setBids(nlohmann::json);
        void setAsks(nlohmann::json);
        void clearRecentTrades();
        void clearStaleOrders(double, double);
        void updateOrders(nlohmann::json);
        void updateTrades(const double&, const double&, bool);

        friend std::ostream& operator<<(std::ostream&, const Market&);
};
