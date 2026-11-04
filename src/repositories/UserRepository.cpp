#include "database/Database.h"
#include "repositories/Repositories.h"

namespace UserRepository {

namespace {
const char* kSelect =
    "SELECT u.user_id, u.login, u.password_hash, u.role_id, u.full_name, u.phone, r.name "
    "FROM users u JOIN roles r ON r.role_id = u.role_id ";

User map(const Statement& st) {
    User u;
    u.id = st.columnInt(0);
    u.login = st.columnText(1);
    u.passwordHash = st.columnText(2);
    u.roleId = st.columnInt(3);
    u.fullName = st.columnText(4);
    u.phone = st.columnOptText(5);
    u.roleName = st.columnText(6);
    return u;
}

std::vector<User> fetchAll(Statement& st) {
    std::vector<User> result;
    while (st.step()) result.push_back(map(st));
    return result;
}
}  // namespace

int create(const std::string& login, const std::string& passwordHash, int roleId,
           const std::string& fullName, const std::optional<std::string>& phone) {
    auto& db = Database::instance();
    Statement st(db.handle(),
                 "INSERT INTO users (login, password_hash, role_id, full_name, phone) "
                 "VALUES (?, ?, ?, ?, ?)");
    st.bind(1, login).bind(2, passwordHash).bind(3, roleId).bind(4, fullName).bind(5, phone);
    st.step();
    return static_cast<int>(db.lastInsertId());
}

std::optional<User> getById(int id) {
    Statement st(Database::instance().handle(), std::string(kSelect) + "WHERE u.user_id = ?");
    st.bind(1, id);
    if (st.step()) return map(st);
    return std::nullopt;
}

std::optional<User> getByLogin(const std::string& login) {
    Statement st(Database::instance().handle(),
                 std::string(kSelect) + "WHERE lower(u.login) = lower(?)");
    st.bind(1, login);
    if (st.step()) return map(st);
    return std::nullopt;
}

std::vector<User> getAll() {
    Statement st(Database::instance().handle(), std::string(kSelect) + "ORDER BY u.user_id");
    return fetchAll(st);
}

std::vector<User> getByRole(int roleId) {
    Statement st(Database::instance().handle(),
                 std::string(kSelect) + "WHERE u.role_id = ? ORDER BY u.full_name");
    st.bind(1, roleId);
    return fetchAll(st);
}

void update(const User& user) {
    Statement st(Database::instance().handle(),
                 "UPDATE users SET login = ?, password_hash = ?, role_id = ?, full_name = ?, "
                 "phone = ? WHERE user_id = ?");
    st.bind(1, user.login)
        .bind(2, user.passwordHash)
        .bind(3, user.roleId)
        .bind(4, user.fullName)
        .bind(5, user.phone)
        .bind(6, user.id);
    st.step();
}

void remove(int id) {
    Statement st(Database::instance().handle(), "DELETE FROM users WHERE user_id = ?");
    st.bind(1, id);
    st.step();
}

}  // namespace UserRepository
