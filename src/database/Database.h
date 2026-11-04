#pragma once
#include <string>
#include "sqlite3.h"

// Подключение к базе данных SQLite (одно на всю программу)
extern sqlite3* db;

bool openDatabase(const std::string& dbPath, const std::string& sqlPath);
void closeDatabase();

// Текст из столбца результата запроса (NULL -> пустая строка)
std::string columnText(sqlite3_stmt* stmt, int col);
