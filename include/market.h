#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <iostream>

class Market {
    private:
        std::string question_;
        std::string tokenId_;
        double bid_;
        double ask_;

    public:
        Market();
        Market(nlohmann::json);

        ~Market() {};

        std::string getQuestion();
        std::string getTokenId();
        double getBid();
        double getAsk();

        void setQuestion(std::string);
        void setTokenId(std::string);
        void setBid(double);
        void setAsk(double);

        friend std::ostream& operator<<(std::ostream&, const Market&);
};

