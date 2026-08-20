#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <iostream>

class Market {
    private:
        std::string question;
        double bid;
        double ask;

    public:
        Market();
        Market(nlohmann::json);
        Market(std::string, double, double);

        ~Market() {};

        std::string getQuestion();
        double getBid();
        double getAsk();

        void setQuestion(std::string);
        void setBid(double);
        void setAsk(double);

        friend std::ostream& operator<<(std::ostream&, const Market&);
};

