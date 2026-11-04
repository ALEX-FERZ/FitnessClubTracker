#pragma once
#include <string>

// role: 0 - клиент, 1 - администратор
const int ROLE_CLIENT = 0;
const int ROLE_ADMIN = 1;

struct User {
    int id;
    std::string login;
    std::string passwordHash;
    int role;
    std::string fullName;
    std::string phone;
};
