#pragma once
#include <optional>
#include <string>
#include "models/User.h"

namespace AuthService {
std::string hashPassword(const std::string& password);
std::optional<User> login(const std::string& login, const std::string& password);
// Регистрация клиента (ROLE_CLIENT) или администратора (ROLE_ADMIN).
// При ошибке бросает std::runtime_error с текстом ошибки.
User registerUser(const std::string& login, const std::string& password, int role,
                  const std::string& fullName, const std::string& phone);
}
