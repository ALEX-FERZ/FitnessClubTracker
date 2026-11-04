#include "database/Database.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

sqlite3* db = nullptr;

bool openDatabase(const std::string& dbPath, const std::string& sqlPath) {
    bool isNew = !std::filesystem::exists(dbPath);
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::cout << "Не удалось открыть БД: " << sqlite3_errmsg(db) << "\n";
        return false;
    }
    sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);

    // Если файла БД нет — создаём таблицы и тестовые данные из database.sql
    if (isNew) {
        std::ifstream file(sqlPath);
        std::stringstream sql;
        sql << file.rdbuf();
        char* error = nullptr;
        if (sqlite3_exec(db, sql.str().c_str(), nullptr, nullptr, &error) != SQLITE_OK) {
            std::cout << "Ошибка создания БД: " << error << "\n";
            sqlite3_free(error);
            return false;
        }
    }
    return true;
}

void closeDatabase() {
    sqlite3_close(db);
    db = nullptr;
}

std::string columnText(sqlite3_stmt* stmt, int col) {
    const unsigned char* text = sqlite3_column_text(stmt, col);
    return text ? reinterpret_cast<const char*>(text) : "";
}
