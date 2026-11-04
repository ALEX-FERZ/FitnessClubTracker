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
    std::cout << "Кого регистрируем?\n"
                 "1. Клиент\n"
                 "2. Администратор\n";
    std::string choice = readLine("Выберите: ");
    int role;
    if (choice == "1") role = ROLE_CLIENT;
    else if (choice == "2") role = ROLE_ADMIN;
    else {
        std::cout << "Нет такого варианта\n";
        return;
    }

    std::string login = readLine("Логин: ");
    std::string password = readLine("Пароль: ");
    std::string fullName = readLine("ФИО: ");
    std::string phone = readLine("Телефон: ");
    try {
        AuthService::registerUser(login, password, role, fullName, phone);
        std::cout << "Регистрация прошла успешно, теперь войдите в систему\n";
    } catch (const std::exception& e) {
        std::cout << "Ошибка: " << e.what() << "\n";
    }
}
