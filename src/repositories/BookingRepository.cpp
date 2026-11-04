#include "database/Database.h"
#include "repositories/Repositories.h"

namespace BookingRepository {

namespace {
const char* kSelect =
    "SELECT b.booking_id, b.client_id, b.lesson_id, b.created_at, b.status_id, "
    "       s.name, u.full_name, l.title, l.starts_at "
    "FROM bookings b "
    "JOIN booking_statuses s ON s.status_id = b.status_id "
    "JOIN users u ON u.user_id = b.client_id "
    "JOIN lessons l ON l.lesson_id = b.lesson_id ";

Booking map(const Statement& st) {
    Booking b;
    b.id = st.columnInt(0);
    b.clientId = st.columnInt(1);
    b.lessonId = st.columnInt(2);
    b.createdAt = st.columnText(3);
    b.statusId = st.columnInt(4);
    b.statusName = st.columnText(5);
    b.clientName = st.columnText(6);
    b.lessonTitle = st.columnText(7);
    b.lessonStartsAt = st.columnText(8);
    return b;
}

std::vector<Booking> fetchAll(Statement& st) {
    std::vector<Booking> result;
    while (st.step()) result.push_back(map(st));
    return result;
}
}  // namespace

int create(int clientId, int lessonId) {
    auto& db = Database::instance();
    Statement st(db.handle(),
                 "INSERT INTO bookings (client_id, lesson_id, created_at, status_id) "
                 "VALUES (?, ?, strftime('%Y-%m-%d %H:%M', 'now', 'localtime'), 1)");
    st.bind(1, clientId).bind(2, lessonId);
    st.step();
    return static_cast<int>(db.lastInsertId());
}

std::optional<Booking> getById(int id) {
    Statement st(Database::instance().handle(), std::string(kSelect) + "WHERE b.booking_id = ?");
    st.bind(1, id);
    if (st.step()) return map(st);
    return std::nullopt;
}

std::vector<Booking> getAll() {
    Statement st(Database::instance().handle(),
                 std::string(kSelect) + "ORDER BY l.starts_at, u.full_name");
    return fetchAll(st);
}

std::vector<Booking> getByClient(int clientId) {
    Statement st(Database::instance().handle(),
                 std::string(kSelect) + "WHERE b.client_id = ? ORDER BY l.starts_at");
    st.bind(1, clientId);
    return fetchAll(st);
}

std::vector<Booking> getByLesson(int lessonId) {
    Statement st(Database::instance().handle(),
                 std::string(kSelect) + "WHERE b.lesson_id = ? ORDER BY u.full_name");
    st.bind(1, lessonId);
    return fetchAll(st);
}

std::optional<Booking> findActive(int clientId, int lessonId) {
    Statement st(Database::instance().handle(),
                 std::string(kSelect) +
                     "WHERE b.client_id = ? AND b.lesson_id = ? AND b.status_id = 1");
    st.bind(1, clientId).bind(2, lessonId);
    if (st.step()) return map(st);
    return std::nullopt;
}

int countActiveByLesson(int lessonId) {
    Statement st(Database::instance().handle(),
                 "SELECT COUNT(*) FROM bookings WHERE lesson_id = ? AND status_id = 1");
    st.bind(1, lessonId);
    st.step();
    return st.columnInt(0);
}

void updateStatus(int id, int statusId) {
    Statement st(Database::instance().handle(),
                 "UPDATE bookings SET status_id = ? WHERE booking_id = ?");
    st.bind(1, statusId).bind(2, id);
    st.step();
}

void remove(int id) {
    Statement st(Database::instance().handle(), "DELETE FROM bookings WHERE booking_id = ?");
    st.bind(1, id);
    st.step();
}

}  // namespace BookingRepository
