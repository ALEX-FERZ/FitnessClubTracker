#include "repositories/TrainerRepository.h"
#include "database/Database.h"

namespace TrainerRepository {

int create(const Trainer& trainer) {
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, "INSERT INTO Trainer (fullName, phone) VALUES (?, ?)", -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, trainer.fullName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, trainer.phone.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (int)sqlite3_last_insert_rowid(db);
}

std::optional<Trainer> getById(int id) {
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, "SELECT TrainerID, fullName, phone FROM Trainer WHERE TrainerID = ?", -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, id);
    std::optional<Trainer> result;
    if (sqlite3_step(stmt) == SQLITE_ROW)
        result = Trainer{sqlite3_column_int(stmt, 0), columnText(stmt, 1), columnText(stmt, 2)};
    sqlite3_finalize(stmt);
    return result;
}

std::vector<Trainer> getAll() {
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, "SELECT TrainerID, fullName, phone FROM Trainer ORDER BY fullName", -1, &stmt, nullptr);
    std::vector<Trainer> result;
    while (sqlite3_step(stmt) == SQLITE_ROW)
        result.push_back({sqlite3_column_int(stmt, 0), columnText(stmt, 1), columnText(stmt, 2)});
    sqlite3_finalize(stmt);
    return result;
}

}
