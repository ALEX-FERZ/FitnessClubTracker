#include <cctype>
#include <cstdint>
#include <cstdio>

#include "repositories/Repositories.h"
#include "services/Services.h"

namespace {
std::string trim(const std::string& s) {
    auto b = s.find_first_not_of(" \t\r\n");
    if (b == std::string::npos) return "";
    auto e = s.find_last_not_of(" \t\r\n");
    return s.substr(b, e - b + 1);
}

void validateLogin(const std::string& login) {
    if (login.size() < 3 || login.size() > 32)
        throw ServiceError("Логин должен содержать от 3 до 32 символов");
    for (unsigned char c : login)
        if (!std::isalnum(c) && c != '_')
            throw ServiceError("Логин может содержать только латинские буквы, цифры и _");
}

void validatePhone(const std::string& phone) {
    if (phone.empty()) return;  // телефон необязателен
    std::size_t start = phone[0] == '+' ? 1 : 0;
    if (phone.size() - start < 10 || phone.size() - start > 15)
        throw ServiceError("Телефон должен содержать от 10 до 15 цифр, например +79001234567");
    for (std::size_t i = start; i < phone.size(); ++i)
        if (!std::isdigit(static_cast<unsigned char>(phone[i])))
            throw ServiceError("Телефон может содержать только цифры и + в начале");
}
}  // namespace

namespace AuthService {

// Учебный хэш FNV-1a (64 бит) от строки "login:password".
// Пароль в открытом виде в БД не хранится. В реальной системе нужен bcrypt/argon2.
std::string hashPassword(const std::string& login, const std::string& password) {
    std::uint64_t h = 0xcbf29ce484222325ULL;
    for (unsigned char c : login + ":" + password) {
        h ^= c;
        h *= 0x100000001b3ULL;
    }
    char buf[17];
    std::snprintf(buf, sizeof(buf), "%016llx", static_cast<unsigned long long>(h));
    return buf;
}

std::optional<User> authenticate(const std::string& login, const std::string& password) {
    auto user = UserRepository::getByLogin(trim(login));
    if (!user || user->passwordHash != hashPassword(user->login, password)) return std::nullopt;
    return user;
}

User registerUser(const std::string& loginRaw, const std::string& password, int roleId,
                  const std::string& fullNameRaw, const std::string& phoneRaw) {
    std::string login = trim(loginRaw), fullName = trim(fullNameRaw), phone = trim(phoneRaw);
    validateLogin(login);
    if (password.size() < 6) throw ServiceError("Пароль должен быть не короче 6 символов");
    if (fullName.empty()) throw ServiceError("ФИО не может быть пустым");
    validatePhone(phone);
    if (roleId != Role::Client && roleId != Role::Trainer && roleId != Role::Admin)
        throw ServiceError("Неизвестная роль");
    if (UserRepository::getByLogin(login)) throw ServiceError("Логин уже занят");

    std::optional<std::string> optPhone;
    if (!phone.empty()) optPhone = phone;
    int id = UserRepository::create(login, hashPassword(login, password), roleId, fullName,
                                    optPhone);
    return *UserRepository::getById(id);
}

User registerClient(const std::string& login, const std::string& password,
                    const std::string& fullName, const std::string& phone) {
    return registerUser(login, password, Role::Client, fullName, phone);
}

}  // namespace AuthService

namespace UserService {

std::vector<User> getClients() { return UserRepository::getByRole(Role::Client); }
std::vector<User> getTrainers() { return UserRepository::getByRole(Role::Trainer); }

User getById(int id) {
    auto user = UserRepository::getById(id);
    if (!user) throw ServiceError("Пользователь не найден");
    return *user;
}

void updateContacts(int userId, const std::string& fullNameRaw, const std::string& phoneRaw) {
    User user = getById(userId);
    std::string fullName = trim(fullNameRaw), phone = trim(phoneRaw);
    if (!fullName.empty()) user.fullName = fullName;
    if (!phone.empty()) {
        if (phone == "-") {
            user.phone.reset();
        } else {
            validatePhone(phone);
            user.phone = phone;
        }
    }
    UserRepository::update(user);
}

void deleteUser(int userId, int currentUserId) {
    User user = getById(userId);
    if (userId == currentUserId) throw ServiceError("Нельзя удалить самого себя");
    if (user.roleId == Role::Trainer && !LessonRepository::getByTrainer(userId).empty())
        throw ServiceError("У тренера есть занятия в расписании — сначала удалите или передайте их");
    // Абонементы и записи клиента удаляются каскадно (ON DELETE CASCADE).
    UserRepository::remove(userId);
}

}  // namespace UserService
