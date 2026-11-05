#include "ui/Menu.h"
#include <iostream>
#include <stdexcept>
#include "constants/Constants.h"
#include "export/Exporter.h"
#include "repositories/UserRepository.h"
#include "services/BookingService.h"
#include "services/LessonService.h"
#include "ui/Auth.h"
#include "ui/Input.h"

namespace {

void printSchedule() {
    std::cout << "\n--- Расписание ---\n";
    for (const Lesson& l : LessonService::getSchedule())
        std::cout << "[" << l.id << "] " << l.startsAt << "  " << l.title << "  (тренер: " << l.trainerName
                  << ", мест: " << l.booked << "/" << l.capacity << ")\n";
}

void printBookings(const std::vector<Booking>& bookings) {
    if (bookings.empty()) std::cout << "Записей нет\n";
    for (const Booking& b : bookings)
        std::cout << "[" << b.id << "] " << b.lessonTime << "  " << b.lessonTitle << "  " << b.clientName
                  << "  — " << (b.status == BOOKING_ACTIVE ? "активна" : "отменена") << "\n";
}

void printClients() {
    std::cout << "\n--- Клиенты ---\n";
    for (const User& u : UserRepository::getByRole(ROLE_CLIENT))
        std::cout << "[" << u.id << "] " << u.fullName << "  " << u.phone << "\n";
}

void printTrainers() {
    std::cout << "\n--- Тренеры ---\n";
    for (const Trainer& t : LessonService::getTrainers())
        std::cout << "[" << t.id << "] " << t.fullName << "  " << t.phone << "\n";
}

void printSubscriptions(int clientId) {
    auto subs = BookingService::getSubscriptions(clientId);
    if (subs.empty()) std::cout << "Абонементов нет\n";
    for (const Subscription& s : subs)
        std::cout << "[" << s.id << "] " << s.type << "  с " << s.startDate << " по " << s.endDate
                  << ", осталось посещений: " << s.visitsLeft << "\n";
}

}

void mainMenu() {
    while (true) {
        std::cout << "\n===== Фитнес-клуб =====\n"
                     "1. Вход\n"
                     "2. Регистрация\n"
                     "0. Выход\n";
        std::string choice = readLine("Выберите действие: ");

        if (choice == "1") {
            auto user = loginMenu();
            if (user && user->role == ROLE_ADMIN) adminMenu(*user);
            else if (user) clientMenu(*user);
        } else if (choice == "2") {
            registerMenu();
        } else if (choice == "0") {
            break;
        }
    }
}

void adminMenu(const User& user) {
    while (true) {
        std::cout << "\n===== Администратор (" << user.login << ") =====\n"
                     "1. Расписание\n"
                     "2. Добавить занятие\n"
                     "3. Изменить занятие\n"
                     "4. Удалить занятие\n"
                     "5. Кто записан на занятие\n"
                     "6. База клиентов и их абонементы\n"
                     "7. Оформить абонемент клиенту\n"
                     "8. Экспорт расписания (JSON, CSV, XML, YAML)\n"
                     "9. База тренеров\n"
                     "10. Добавить тренера\n"
                     "11. Зарегистрировать клиента или администратора\n"
                     "0. Выход\n";
        std::string choice = readLine("Выберите действие: ");

        try {
            if (choice == "1") {
                printSchedule();
            } else if (choice == "2") {
                printTrainers();
                int trainerId = readInt("Номер тренера: ");
                std::string title = readLine("Название: ");
                std::string time = readLine("Дата и время (ГГГГ-ММ-ДД ЧЧ:ММ): ");
                int places = readInt("Количество мест: ");
                LessonService::addLesson(trainerId, title, time, places);
                std::cout << "Занятие добавлено\n";
            } else if (choice == "3") {
                printSchedule();
                Lesson l = LessonService::getById(readInt("Номер занятия: "));
                std::string title = readLine("Новое название (Enter — оставить): ");
                std::string time = readLine("Новые дата и время (Enter — оставить): ");
                std::string places = readLine("Новое количество мест (Enter — оставить): ");
                LessonService::editLesson(l.id, title.empty() ? l.title : title, time.empty() ? l.startsAt : time,
                                          places.empty() ? l.capacity : std::stoi(places));
                std::cout << "Занятие изменено\n";
            } else if (choice == "4") {
                printSchedule();
                LessonService::deleteLesson(readInt("Номер занятия: "));
                std::cout << "Занятие удалено\n";
            } else if (choice == "5") {
                printSchedule();
                printBookings(LessonService::getBookings(readInt("Номер занятия: ")));
            } else if (choice == "6") {
                printClients();
                printSubscriptions(readInt("Номер клиента: "));
            } else if (choice == "7") {
                printClients();
                int clientId = readInt("Номер клиента: ");
                std::string type = readLine("Тип абонемента: ");
                std::string start = readLine("Дата начала (ГГГГ-ММ-ДД): ");
                std::string end = readLine("Дата окончания (ГГГГ-ММ-ДД): ");
                int visits = readInt("Количество посещений: ");
                BookingService::addSubscription(clientId, type, start, end, visits);
                std::cout << "Абонемент оформлен\n";
            } else if (choice == "8") {
                Exporter::exportLessons(OUT_DIR);
            } else if (choice == "9") {
                printTrainers();
            } else if (choice == "10") {
                std::string name = readLine("ФИО тренера: ");
                std::string phone = readLine("Телефон: ");
                LessonService::addTrainer(name, phone);
                std::cout << "Тренер добавлен\n";
            } else if (choice == "11") {
                registerMenu();
            } else if (choice == "0") {
                break;
            }
        } catch (const std::exception& e) {
            std::cout << "Ошибка: " << e.what() << "\n";
        }
    }
}

void clientMenu(const User& user) {
    while (true) {
        std::cout << "\n===== Клиент (" << user.login << ") =====\n"
                     "1. Расписание\n"
                     "2. Записаться на занятие\n"
                     "3. Мои записи\n"
                     "4. Мои абонементы\n"
                     "0. Выход\n";
        std::string choice = readLine("Выберите действие: ");

        try {
            if (choice == "1") {
                printSchedule();
            } else if (choice == "2") {
                printSchedule();
                BookingService::book(user.id, readInt("Номер занятия: "));
                std::cout << "Вы записаны на занятие\n";
            } else if (choice == "3") {
                printBookings(BookingService::getClientBookings(user.id));
            } else if (choice == "4") {
                printSubscriptions(user.id);
            } else if (choice == "0") {
                break;
            }
        } catch (const std::exception& e) {
            std::cout << "Ошибка: " << e.what() << "\n";
        }
    }
}
