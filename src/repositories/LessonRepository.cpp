#include "database/Database.h"
#include "repositories/Repositories.h"

namespace LessonRepository {

namespace {
// Вместе с занятием сразу получаем ФИО тренера и число активных записей.
const char* kSelect =
    "SELECT l.lesson_id, l.trainer_id, l.title, l.starts_at, l.capacity, u.full_name, "
    "       (SELECT COUNT(*) FROM bookings b "
    "         WHERE b.lesson_id = l.lesson_id AND b.status_id = 1) "
    "FROM lessons l JOIN users u ON u.user_id = l.trainer_id ";

Lesson map(const Statement& st) {
    Lesson l;
    l.id = st.columnInt(0);
    l.trainerId = st.columnInt(1);
    l.title = st.columnText(2);
    l.startsAt = st.columnText(3);
    l.capacity = st.columnInt(4);
    l.trainerName = st.columnText(5);
    l.bookedCount = st.columnInt(6);
    return l;
}

std::vector<Lesson> fetchAll(Statement& st) {
    std::vector<Lesson> result;
    while (st.step()) result.push_back(map(st));
    return result;
}
}  // namespace

int create(int trainerId, const std::string& title, const std::string& startsAt, int capacity) {
    auto& db = Database::instance();
    Statement st(db.handle(),
                 "INSERT INTO lessons (trainer_id, title, starts_at, capacity) VALUES (?, ?, ?, ?)");
    st.bind(1, trainerId).bind(2, title).bind(3, startsAt).bind(4, capacity);
    st.step();
    return static_cast<int>(db.lastInsertId());
}

std::optional<Lesson> getById(int id) {
    Statement st(Database::instance().handle(), std::string(kSelect) + "WHERE l.lesson_id = ?");
    st.bind(1, id);
    if (st.step()) return map(st);
    return std::nullopt;
}

std::vector<Lesson> getAll() {
    Statement st(Database::instance().handle(), std::string(kSelect) + "ORDER BY l.starts_at");
    return fetchAll(st);
}

std::vector<Lesson> getByTrainer(int trainerId) {
    Statement st(Database::instance().handle(),
                 std::string(kSelect) + "WHERE l.trainer_id = ? ORDER BY l.starts_at");
    st.bind(1, trainerId);
    return fetchAll(st);
}

void update(const Lesson& lesson) {
    Statement st(Database::instance().handle(),
                 "UPDATE lessons SET trainer_id = ?, title = ?, starts_at = ?, capacity = ? "
                 "WHERE lesson_id = ?");
    st.bind(1, lesson.trainerId)
        .bind(2, lesson.title)
        .bind(3, lesson.startsAt)
        .bind(4, lesson.capacity)
        .bind(5, lesson.id);
    st.step();
}

void remove(int id) {
    Statement st(Database::instance().handle(), "DELETE FROM lessons WHERE lesson_id = ?");
    st.bind(1, id);
    st.step();
}

}  // namespace LessonRepository
