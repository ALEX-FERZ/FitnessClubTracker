#pragma once
// Пользовательский интерфейс: авторизация и меню для каждой роли.
// UI не обращается к БД напрямую — только к сервисам.

#include <optional>
#include <string>

#include "models/Models.h"

struct AppConfig {
    std::string dbPath = "database/database.db";
    std::string schemaPath = "database/schema.sql";
    std::string outDir = "out";
};

namespace AuthUi {
std::optional<User> login();
void registerClient();
}  // namespace AuthUi

namespace MenuUi {
void mainMenu(const AppConfig& config);
void adminMenu(const User& user, const AppConfig& config);
void trainerMenu(const User& user);
void clientMenu(const User& user);
}  // namespace MenuUi
