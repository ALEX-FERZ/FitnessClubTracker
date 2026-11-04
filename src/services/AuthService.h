#pragma once
#include <optional>
#include <string>
#include "models/User.h"

namespace AuthService {
std::string hashPassword(const std::string& password);
std::optional<User> login(const std::string& login, const std::string& password);
// Регистрация клиента. При ошибке бросает std::runtime_error с текстом ошибки.
User registerClient(const std::string& login, const std::string& password,
                    const std::string& fullName, const std::string& phone);
}
