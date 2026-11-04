#include "ui/Auth.h"
#include <iostream>
#include <stdexcept>
#include "services/AuthService.h"
#include "ui/Input.h"

std::optional<User> loginMenu() {
    std::string login = readLine("Логин: ");
    std::string password = readLine("Пароль: ");
    auto user = AuthService::login(login, password);
    if (!user) {
        std::cout << "Неверный логин или пароль\n";
        return std::nullopt;
    }
    std::cout << "Здравствуйте, " << user->fullName << "!\n";
    return user;
}

void registerMenu() {
    std::string login = readLine("Логин: ");
    std::string password = readLine("Пароль: ");
    std::string fullName = readLine("ФИО: ");
    std::string phone = readLine("Телефон: ");
    try {
        AuthService::registerClient(login, password, fullName, phone);
        std::cout << "Регистрация прошла успешно, теперь войдите в систему\n";
    } catch (const std::exception& e) {
        std::cout << "Ошибка: " << e.what() << "\n";
    }
}
