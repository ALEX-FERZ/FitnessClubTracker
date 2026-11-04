#include "database/Database.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

// ---------------- Statement ----------------

Statement::Statement(sqlite3* db, const std::string& sql) : db_(db) {
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt_, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("Ошибка SQL: ") + sqlite3_errmsg(db_));
    }
}

Statement::~Statement() { sqlite3_finalize(stmt_); }

Statement& Statement::bind(int index, int value) {
    sqlite3_bind_int(stmt_, index, value);
    return *this;
}

Statement& Statement::bind(int index, const std::string& value) {
    sqlite3_bind_text(stmt_, index, value.c_str(), -1, SQLITE_TRANSIENT);
    return *this;
}

Statement& Statement::bind(int index, const std::optional<std::string>& value) {
    return value ? bind(index, *value) : bindNull(index);
}

Statement& Statement::bindNull(int index) {
    sqlite3_bind_null(stmt_, index);
    return *this;
}

bool Statement::step() {
    int rc = sqlite3_step(stmt_);
    if (rc == SQLITE_ROW) return true;
    if (rc == SQLITE_DONE) return false;
    throw std::runtime_error(std::string("Ошибка БД: ") + sqlite3_errmsg(db_));
}

int Statement::columnCount() const { return sqlite3_column_count(stmt_); }
std::string Statement::columnName(int col) const { return sqlite3_column_name(stmt_, col); }
int Statement::columnType(int col) const { return sqlite3_column_type(stmt_, col); }
bool Statement::isNull(int col) const { return columnType(col) == SQLITE_NULL; }
int Statement::columnInt(int col) const { return sqlite3_column_int(stmt_, col); }
double Statement::columnDouble(int col) const { return sqlite3_column_double(stmt_, col); }

std::string Statement::columnText(int col) const {
    const unsigned char* text = sqlite3_column_text(stmt_, col);
    return text ? reinterpret_cast<const char*>(text) : "";
}

std::optional<std::string> Statement::columnOptText(int col) const {
    if (isNull(col)) return std::nullopt;
    return columnText(col);
}

// ---------------- Database ----------------

Database& Database::instance() {
    static Database db;
    return db;
}

Database::~Database() { close(); }

void Database::open(const std::string& dbPath, const std::string& schemaPath) {
    close();
    bool isNew = !std::filesystem::exists(dbPath);
    if (sqlite3_open(dbPath.c_str(), &db_) != SQLITE_OK) {
        std::string msg = sqlite3_errmsg(db_);
        close();
        throw std::runtime_error("Не удалось открыть БД " + dbPath + ": " + msg);
    }
    exec("PRAGMA foreign_keys = ON;");
    if (isNew) reset(schemaPath);
}

void Database::reset(const std::string& schemaPath) { exec(readTextFile(schemaPath)); }

void Database::close() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

sqlite3* Database::handle() const {
    if (!db_) throw std::runtime_error("База данных не открыта");
    return db_;
}

void Database::exec(const std::string& sql) {
    char* err = nullptr;
    if (sqlite3_exec(handle(), sql.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
        std::string msg = err ? err : "неизвестная ошибка";
        sqlite3_free(err);
        throw std::runtime_error("Ошибка SQL: " + msg);
    }
}

long long Database::lastInsertId() const { return sqlite3_last_insert_rowid(handle()); }
int Database::changes() const { return sqlite3_changes(handle()); }

std::string readTextFile(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw std::runtime_error("Не найден файл: " + path);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}
