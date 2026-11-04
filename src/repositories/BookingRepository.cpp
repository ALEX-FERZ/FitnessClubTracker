#include "repositories/BookingRepository.h"
#include "database/Database.h"

namespace {
const std::string SELECT_BOOKING =
    "SELECT b.BookingID, b.ClientID, b.LessonID, b.createdAt, b.status, u.fullName, l.title, l.startTime "
    "FROM Booking b JOIN User u ON u.UserID = b.ClientID JOIN Lesson l ON l.LessonID = b.LessonID ";

Booking map(sqlite3_stmt* stmt) {
    return Booking{sqlite3_column_int(stmt, 0), sqlite3_column_int(stmt, 1), sqlite3_column_int(stmt, 2),
                   columnText(stmt, 3), sqlite3_column_int(stmt, 4), columnText(stmt, 5),
                   columnText(stmt, 6), columnText(stmt, 7)};
}

std::vector<Booking> getWhere(const std::string& where, int id) {
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, (SELECT_BOOKING + where + " ORDER BY l.startTime").c_str(), -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, id);
    std::vector<Booking> result;
    while (sqlite3_step(stmt) == SQLITE_ROW) result.push_back(map(stmt));
    sqlite3_finalize(stmt);
    return result;
}
}

namespace BookingRepository {

int create(int clientId, int lessonId) {
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db,
        "INSERT INTO Booking (ClientID, LessonID, createdAt, status) "
        "VALUES (?, ?, datetime('now', 'localtime'), 1)",
        -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, clientId);
    sqlite3_bind_int(stmt, 2, lessonId);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (int)sqlite3_last_insert_rowid(db);
}

std::optional<Booking> getById(int id) {
    auto list = getWhere("WHERE b.BookingID = ?", id);
    if (list.empty()) return std::nullopt;
    return list[0];
}

std::vector<Booking> getByClient(int clientId) { return getWhere("WHERE b.ClientID = ?", clientId); }
std::vector<Booking> getByLesson(int lessonId) { return getWhere("WHERE b.LessonID = ?", lessonId); }

void updateStatus(int id, int status) {
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, "UPDATE Booking SET status = ? WHERE BookingID = ?", -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, status);
    sqlite3_bind_int(stmt, 2, id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

}
