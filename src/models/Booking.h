#pragma once
#include <string>
#include "constants/Constants.h"

struct Booking {
    int id;
    int clientId;
    int lessonId;
    std::string createdAt;
    int status;

    // дополнительные поля (в таблице Booking их нет)
    std::string clientName;
    std::string lessonTitle;
    std::string lessonTime;
};
