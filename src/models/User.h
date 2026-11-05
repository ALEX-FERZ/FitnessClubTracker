#pragma once
#include <string>
#include "constants/Constants.h"

struct User {
    int id;
    std::string login;
    std::string passwordHash;
    int role;
    std::string fullName;
    std::string phone;
};
