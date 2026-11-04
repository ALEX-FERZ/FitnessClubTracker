#include "services/Services.h"
#include "ui/Console.h"
#include "ui/Menu.h"

namespace AuthUi {

std::optional<User> login() {
    Console::title("Вход в систему");
    std::string loginValue = Console::readLine("Логин: ");
    std::string password = Console::readLine("Пароль: ");
    auto user = AuthService::authenticate(loginValue, password);
    if (!user) {
        Console::error("Неверный логин или пароль");
        return std::nullopt;
    }
    Console::success("Добро пожаловать, " + user->fullName + " (" + user->roleName + ")");
    return user;
}

void registerClient() {
    Console::title("Регистрация клиента");
    std::string loginValue = Console::readLine("Логин (латиница, цифры, _): ");
    std::string password = Console::readLine("Пароль (от 6 символов): ");
    std::string fullName = Console::readLine("ФИО: ");
    std::string phone = Console::readLine("Телефон (необязательно, +7...): ");
    try {
        User user = AuthService::registerClient(loginValue, password, fullName, phone);
        Console::success("Клиент " + user.login + " зарегистрирован. Теперь войдите в систему.");
        Console::success("Чтобы записываться на занятия, оформите абонемент у администратора.");
    } catch (const ServiceError& e) {
        Console::error(e.what());
    }
}

}  // namespace AuthUi
