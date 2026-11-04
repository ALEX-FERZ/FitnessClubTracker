#include "database/Database.h"
#include "repositories/Repositories.h"

namespace SubscriptionRepository {

namespace {
const char* kSelect =
    "SELECT s.subscription_id, s.client_id, s.type, s.start_date, s.end_date, s.visits_left, "
    "       u.full_name "
    "FROM subscriptions s JOIN users u ON u.user_id = s.client_id ";

Subscription map(const Statement& st) {
    Subscription s;
    s.id = st.columnInt(0);
    s.clientId = st.columnInt(1);
    s.type = st.columnText(2);
    s.startDate = st.columnText(3);
    s.endDate = st.columnText(4);
    s.visitsLeft = st.columnInt(5);
    s.clientName = st.columnText(6);
    return s;
}

std::vector<Subscription> fetchAll(Statement& st) {
    std::vector<Subscription> result;
    while (st.step()) result.push_back(map(st));
    return result;
}
}  // namespace

int create(int clientId, const std::string& type, const std::string& startDate,
           const std::string& endDate, int visitsLeft) {
    auto& db = Database::instance();
    Statement st(db.handle(),
                 "INSERT INTO subscriptions (client_id, type, start_date, end_date, visits_left) "
                 "VALUES (?, ?, ?, ?, ?)");
    st.bind(1, clientId).bind(2, type).bind(3, startDate).bind(4, endDate).bind(5, visitsLeft);
    st.step();
    return static_cast<int>(db.lastInsertId());
}

std::optional<Subscription> getById(int id) {
    Statement st(Database::instance().handle(),
                 std::string(kSelect) + "WHERE s.subscription_id = ?");
    st.bind(1, id);
    if (st.step()) return map(st);
    return std::nullopt;
}

std::vector<Subscription> getAll() {
    Statement st(Database::instance().handle(),
                 std::string(kSelect) + "ORDER BY u.full_name, s.start_date");
    return fetchAll(st);
}

std::vector<Subscription> getByClient(int clientId) {
    Statement st(Database::instance().handle(),
                 std::string(kSelect) + "WHERE s.client_id = ? ORDER BY s.end_date DESC");
    st.bind(1, clientId);
    return fetchAll(st);
}

void updateVisits(int id, int visitsLeft) {
    Statement st(Database::instance().handle(),
                 "UPDATE subscriptions SET visits_left = ? WHERE subscription_id = ?");
    st.bind(1, visitsLeft).bind(2, id);
    st.step();
}

void remove(int id) {
    Statement st(Database::instance().handle(),
                 "DELETE FROM subscriptions WHERE subscription_id = ?");
    st.bind(1, id);
    st.step();
}

}  // namespace SubscriptionRepository
