#pragma once
#include <string>

// status: 0 - отменена, 1 - активна
const int BOOKING_CANCELLED = 0;
const int BOOKING_ACTIVE = 1;

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
