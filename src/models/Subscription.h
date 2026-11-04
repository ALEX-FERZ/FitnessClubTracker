#pragma once
#include <string>

struct Subscription {
    int id;
    int clientId;
    std::string type;
    std::string startDate;
    std::string endDate;
    int visitsLeft;
};
