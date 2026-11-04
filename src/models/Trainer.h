#pragma once
#include <string>

// Тренер не входит в систему: его добавляет администратор и прикрепляет к занятиям
struct Trainer {
    int id;
    std::string fullName;
    std::string phone;
};
