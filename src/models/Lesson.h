#pragma once
#include <string>

struct Lesson {
    int id;
    int trainerId;
    std::string title;
    std::string startsAt;
    int capacity;

    // дополнительные поля (в таблице Lesson их нет)
    std::string trainerName;
    int booked = 0;
};
