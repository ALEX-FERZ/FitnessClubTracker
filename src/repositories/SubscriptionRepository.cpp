#include "repositories/SubscriptionRepository.h"
#include "database/Database.h"

namespace SubscriptionRepository {

int create(const Subscription& sub) {
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db,
        "INSERT INTO Subscription (ClientID, type, startDate, endDate, visitsLeft) VALUES (?, ?, ?, ?, ?)",
        -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, sub.clientId);
    sqlite3_bind_text(stmt, 2, sub.type.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, sub.startDate.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, sub.endDate.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, sub.visitsLeft);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (int)sqlite3_last_insert_rowid(db);
}

std::vector<Subscription> getByClient(int clientId) {
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db,
        "SELECT SubscriptionID, ClientID, type, startDate, endDate, visitsLeft "
        "FROM Subscription WHERE ClientID = ? ORDER BY endDate DESC",
        -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, clientId);
    std::vector<Subscription> result;
    while (sqlite3_step(stmt) == SQLITE_ROW)
        result.push_back({sqlite3_column_int(stmt, 0), sqlite3_column_int(stmt, 1), columnText(stmt, 2),
                          columnText(stmt, 3), columnText(stmt, 4), sqlite3_column_int(stmt, 5)});
    sqlite3_finalize(stmt);
    return result;
}

void updateVisits(int id, int visitsLeft) {
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, "UPDATE Subscription SET visitsLeft = ? WHERE SubscriptionID = ?", -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, visitsLeft);
    sqlite3_bind_int(stmt, 2, id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void remove(int id) {
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, "DELETE FROM Subscription WHERE SubscriptionID = ?", -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

}
