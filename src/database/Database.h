#pragma once
// Слой доступа к БД: подключение к SQLite и обёртка над подготовленными запросами.

#include <optional>
#include <string>

#include "sqlite3.h"

// RAII-обёртка над sqlite3_stmt. Параметры передаются через ?, а не конкатенацией,
// что защищает от SQL-инъекций.
class Statement {
public:
    Statement(sqlite3* db, const std::string& sql);
    ~Statement();
    Statement(const Statement&) = delete;
    Statement& operator=(const Statement&) = delete;

    Statement& bind(int index, int value);
    Statement& bind(int index, const std::string& value);
    Statement& bind(int index, const std::optional<std::string>& value);
    Statement& bindNull(int index);

    // Выполняет шаг запроса. true — получена очередная строка результата.
    bool step();

    int columnCount() const;
    std::string columnName(int col) const;
    int columnType(int col) const;
    bool isNull(int col) const;
    int columnInt(int col) const;
    double columnDouble(int col) const;
    std::string columnText(int col) const;
    std::optional<std::string> columnOptText(int col) const;

private:
    sqlite3* db_;
    sqlite3_stmt* stmt_ = nullptr;
};

class Database {
public:
    // Единственное подключение приложения (аналог get_connection()).
    static Database& instance();

    // Открывает файл БД. Если файла ещё нет — создаёт его и выполняет schema.sql.
    void open(const std::string& dbPath, const std::string& schemaPath);
    // Полностью пересоздаёт БД из schema.sql (удаляет все данные).
    void reset(const std::string& schemaPath);
    void close();

    sqlite3* handle() const;
    void exec(const std::string& sql);
    long long lastInsertId() const;
    int changes() const;

private:
    Database() = default;
    ~Database();
    sqlite3* db_ = nullptr;
};

std::string readTextFile(const std::string& path);
