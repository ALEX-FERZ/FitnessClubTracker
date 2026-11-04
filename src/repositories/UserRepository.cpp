#include "repositories/UserRepository.h"
#include "database/Database.h"

namespace {
const std::string SELECT_USER = "SELECT UserID, login, passwordHash, role, fullName, phone FROM User ";

User map(sqlite3_stmt* stmt) {
    return User{sqlite3_column_int(stmt, 0), columnText(stmt, 1), columnText(stmt, 2),
                sqlite3_column_int(stmt, 3), columnText(stmt, 4), columnText(stmt, 5)};
}
}

namespace UserRepository {

int create(const User& user) {
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db,
        "INSERT INTO User (login, passwordHash, role, fullName, phone) VALUES (?, ?, ?, ?, ?)",
        -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, user.login.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, user.passwordHash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, user.role);
    sqlite3_bind_text(stmt, 4, user.fullName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, user.phone.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (int)sqlite3_last_insert_rowid(db);
}

std::optional<User> getById(int id) {
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, (SELECT_USER + "WHERE UserID = ?").c_str(), -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, id);
    std::optional<User> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) result = map(stmt);
    sqlite3_finalize(stmt);
    return result;
}

std::optional<User> getByLogin(const std::string& login) {
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, (SELECT_USER + "WHERE login = ?").c_str(), -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, login.c_str(), -1, SQLITE_TRANSIENT);
    std::optional<User> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) result = map(stmt);
    sqlite3_finalize(stmt);
    return result;
}

std::vector<User> getByRole(int role) {
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, (SELECT_USER + "WHERE role = ? ORDER BY fullName").c_str(), -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, role);
    std::vector<User> result;
    while (sqlite3_step(stmt) == SQLITE_ROW) result.push_back(map(stmt));
    sqlite3_finalize(stmt);
    return result;
}

void update(const User& user) {
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, "UPDATE User SET fullName = ?, phone = ? WHERE UserID = ?", -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, user.fullName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, user.phone.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, user.id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void remove(int id) {
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, "DELETE FROM User WHERE UserID = ?", -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

}
