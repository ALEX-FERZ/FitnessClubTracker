#include "repositories/LessonRepository.h"
#include "database/Database.h"

namespace {
// Вместе с занятием получаем имя тренера и количество активных записей
const std::string SELECT_LESSON =
    "SELECT l.LessonID, l.TrainerID, l.title, l.startTime, l.maxPlaces, u.fullName, "
    "(SELECT COUNT(*) FROM Booking b WHERE b.LessonID = l.LessonID AND b.status = 1) "
    "FROM Lesson l JOIN Trainer u ON u.TrainerID = l.TrainerID ";

Lesson map(sqlite3_stmt* stmt) {
    return Lesson{sqlite3_column_int(stmt, 0), sqlite3_column_int(stmt, 1), columnText(stmt, 2),
                  columnText(stmt, 3), sqlite3_column_int(stmt, 4), columnText(stmt, 5),
                  sqlite3_column_int(stmt, 6)};
}
}

namespace LessonRepository {

int create(const Lesson& lesson) {
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db,
        "INSERT INTO Lesson (TrainerID, title, startTime, maxPlaces) VALUES (?, ?, ?, ?)",
        -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, lesson.trainerId);
    sqlite3_bind_text(stmt, 2, lesson.title.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, lesson.startsAt.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, lesson.capacity);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (int)sqlite3_last_insert_rowid(db);
}

std::optional<Lesson> getById(int id) {
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, (SELECT_LESSON + "WHERE l.LessonID = ?").c_str(), -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, id);
    std::optional<Lesson> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) result = map(stmt);
    sqlite3_finalize(stmt);
    return result;
}

std::vector<Lesson> getAll() {
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, (SELECT_LESSON + "ORDER BY l.startTime").c_str(), -1, &stmt, nullptr);
    std::vector<Lesson> result;
    while (sqlite3_step(stmt) == SQLITE_ROW) result.push_back(map(stmt));
    sqlite3_finalize(stmt);
    return result;
}

void update(const Lesson& lesson) {
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db,
        "UPDATE Lesson SET title = ?, startTime = ?, maxPlaces = ? WHERE LessonID = ?",
        -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, lesson.title.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, lesson.startsAt.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, lesson.capacity);
    sqlite3_bind_int(stmt, 4, lesson.id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void remove(int id) {
    sqlite3_stmt* stmt;
    // сначала удаляем записи на это занятие, иначе не даст внешний ключ
    sqlite3_prepare_v2(db, "DELETE FROM Booking WHERE LessonID = ?", -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    sqlite3_prepare_v2(db, "DELETE FROM Lesson WHERE LessonID = ?", -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

}
