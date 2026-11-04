#include "services/AuthService.h"
#include <stdexcept>
#include "repositories/UserRepository.h"

namespace AuthService {

// Учебный «хэш»: в тестовых данных пароли хранятся как hash_<пароль>
// (например, у ivanov passwordHash = 'hash_1', значит пароль — 1).
std::string hashPassword(const std::string& password) {
    return "hash_" + password;
}

std::optional<User> login(const std::string& login, const std::string& password) {
    auto user = UserRepository::getByLogin(login);
    if (user && user->passwordHash == hashPassword(password)) return user;
    return std::nullopt;
}

User registerClient(const std::string& login, const std::string& password,
                    const std::string& fullName, const std::string& phone) {
    if (login.empty()) throw std::runtime_error("Логин не может быть пустым");
    if (password.size() < 4) throw std::runtime_error("Пароль должен быть не короче 4 символов");
    if (fullName.empty()) throw std::runtime_error("ФИО не может быть пустым");
    if (UserRepository::getByLogin(login)) throw std::runtime_error("Такой логин уже занят");

    User user{0, login, hashPassword(password), ROLE_CLIENT, fullName, phone};
    user.id = UserRepository::create(user);
    return user;
}

}
