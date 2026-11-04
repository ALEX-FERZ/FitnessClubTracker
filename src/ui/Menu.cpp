#include "ui/Menu.h"

#include <functional>
#include <iostream>

#include "database/Database.h"
#include "export/Exporter.h"
#include "services/Services.h"
#include "ui/Console.h"

namespace {

// Выполняет действие и показывает ошибку бизнес-логики, не завершая программу.
void run(const std::function<void()>& action) {
    try {
        action();
    } catch (const ServiceError& e) {
        Console::error(e.what());
    } catch (const std::runtime_error& e) {
        Console::error(e.what());
    }
}

std::string phoneOf(const User& u) { return u.phone.value_or("—"); }

void printLessons(const std::vector<Lesson>& lessons) {
    std::vector<std::vector<std::string>> rows;
    for (auto& l : lessons)
        rows.push_back({std::to_string(l.id), l.startsAt, l.title, l.trainerName,
                        std::to_string(l.bookedCount) + "/" + std::to_string(l.capacity),
                        std::to_string(l.freePlaces())});
    Console::table({"ID", "Дата и время", "Занятие", "Тренер", "Записано", "Свободно"}, rows);
}

void printUsers(const std::vector<User>& users) {
    std::vector<std::vector<std::string>> rows;
    for (auto& u : users)
        rows.push_back({std::to_string(u.id), u.login, u.fullName, phoneOf(u), u.roleName});
    Console::table({"ID", "Логин", "ФИО", "Телефон", "Роль"}, rows);
}

void printBookings(const std::vector<Booking>& bookings) {
    std::vector<std::vector<std::string>> rows;
    for (auto& b : bookings)
        rows.push_back({std::to_string(b.id), b.lessonStartsAt, b.lessonTitle, b.clientName,
                        b.statusName, b.createdAt});
    Console::table({"ID", "Занятие в", "Занятие", "Клиент", "Статус", "Создана"}, rows);
}

void printSubscriptions(const std::vector<Subscription>& subs) {
    std::vector<std::vector<std::string>> rows;
    std::string today = DateTime::today();
    for (auto& s : subs) {
        std::string state = s.endDate < today ? "истёк"
                            : s.startDate > today ? "не начался"
                            : s.visitsLeft == 0  ? "нет посещений"
                                                 : "действует";
        rows.push_back({std::to_string(s.id), s.clientName, s.type, s.startDate, s.endDate,
                        std::to_string(s.visitsLeft), state});
    }
    Console::table({"ID", "Клиент", "Тип", "С", "По", "Осталось", "Состояние"}, rows);
}

void filterSchedule(bool onlyUpcoming) {
    std::string date = Console::readLine("Дата ГГГГ-ММ-ДД (Enter — любая): ");
    printUsers(UserService::getTrainers());
    auto trainerId = Console::readOptionalInt("ID тренера (Enter — любой): ");
    std::optional<std::string> optDate;
    if (!date.empty()) optDate = date;
    auto lessons = LessonService::getSchedule(optDate, trainerId);
    if (onlyUpcoming) {
        std::vector<Lesson> upcoming;
        for (auto& l : lessons)
            if (l.startsAt > DateTime::now()) upcoming.push_back(l);
        lessons = upcoming;
    }
    printLessons(lessons);
}

void showLessonClients() {
    int id = Console::readInt("ID занятия: ");
    Lesson l = LessonService::getById(id);
    std::cout << l.title << " — " << l.startsAt << ", тренер " << l.trainerName << "\n";
    printBookings(BookingService::getForLesson(id));
}

// ---------------- Администратор ----------------

void scheduleMenu() {
    while (true) {
        Console::title("Расписание");
        std::cout << "1. Показать всё расписание\n"
                     "2. Фильтр по дате и тренеру\n"
                     "3. Добавить занятие\n"
                     "4. Изменить занятие\n"
                     "5. Удалить занятие\n"
                     "6. Список записавшихся на занятие\n"
                     "0. Назад\n";
        std::string c = Console::readLine("Выберите действие: ");
        if (c == "1") run([] { printLessons(LessonService::getSchedule()); });
        else if (c == "2") run([] { filterSchedule(false); });
        else if (c == "3") run([] {
            printUsers(UserService::getTrainers());
            int trainerId = Console::readInt("ID тренера: ");
            std::string title = Console::readLine("Название: ");
            std::string startsAt = Console::readLine("Начало (ГГГГ-ММ-ДД ЧЧ:ММ): ");
            int capacity = Console::readInt("Количество мест: ");
            Lesson l = LessonService::createLesson(trainerId, title, startsAt, capacity);
            Console::success("Занятие добавлено, ID " + std::to_string(l.id));
        });
        else if (c == "4") run([] {
            printLessons(LessonService::getSchedule());
            Lesson l = LessonService::getById(Console::readInt("ID занятия: "));
            std::cout << "Enter — оставить текущее значение\n";
            auto trainerId = Console::readOptionalInt("ID тренера [" + std::to_string(l.trainerId) + "]: ");
            std::string title = Console::readLine("Название [" + l.title + "]: ");
            std::string startsAt = Console::readLine("Начало [" + l.startsAt + "]: ");
            auto capacity = Console::readOptionalInt("Мест [" + std::to_string(l.capacity) + "]: ");
            LessonService::updateLesson(l.id, trainerId.value_or(l.trainerId),
                                        title.empty() ? l.title : title,
                                        startsAt.empty() ? l.startsAt : startsAt,
                                        capacity.value_or(l.capacity));
            Console::success("Занятие изменено");
        });
        else if (c == "5") run([] {
            printLessons(LessonService::getSchedule());
            int id = Console::readInt("ID занятия: ");
            Lesson l = LessonService::getById(id);
            if (Console::confirm("Удалить «" + l.title + "» " + l.startsAt + "? Записи будут удалены")) {
                LessonService::deleteLesson(id);
                Console::success("Занятие удалено");
            }
        });
        else if (c == "6") run([] {
            printLessons(LessonService::getSchedule());
            showLessonClients();
        });
        else if (c == "0") return;
    }
}

void editContacts(const std::vector<User>& list) {
    printUsers(list);
    User u = UserService::getById(Console::readInt("ID: "));
    std::cout << "Enter — оставить без изменений, «-» — удалить телефон\n";
    std::string name = Console::readLine("ФИО [" + u.fullName + "]: ");
    std::string phone = Console::readLine("Телефон [" + phoneOf(u) + "]: ");
    UserService::updateContacts(u.id, name, phone);
    Console::success("Данные обновлены");
}

void addUser(int roleId) {
    std::string login = Console::readLine("Логин: ");
    std::string password = Console::readLine("Пароль (от 6 символов): ");
    std::string name = Console::readLine("ФИО: ");
    std::string phone = Console::readLine("Телефон (необязательно): ");
    User u = AuthService::registerUser(login, password, roleId, name, phone);
    Console::success(u.roleName + " " + u.fullName + " добавлен(а), ID " + std::to_string(u.id));
}

void deleteUser(const std::vector<User>& list, int currentUserId) {
    printUsers(list);
    User u = UserService::getById(Console::readInt("ID: "));
    if (Console::confirm("Удалить " + u.fullName + "?")) {
        UserService::deleteUser(u.id, currentUserId);
        Console::success("Удалено");
    }
}

void trainersMenu(const User& admin) {
    while (true) {
        Console::title("Тренеры");
        std::cout << "1. Список тренеров\n2. Добавить тренера\n3. Изменить контакты\n"
                     "4. Удалить тренера\n0. Назад\n";
        std::string c = Console::readLine("Выберите действие: ");
        if (c == "1") run([] { printUsers(UserService::getTrainers()); });
        else if (c == "2") run([] { addUser(Role::Trainer); });
        else if (c == "3") run([] { editContacts(UserService::getTrainers()); });
        else if (c == "4") run([&] { deleteUser(UserService::getTrainers(), admin.id); });
        else if (c == "0") return;
    }
}

void clientsMenu(const User& admin) {
    while (true) {
        Console::title("Клиенты и абонементы");
        std::cout << "1. Список клиентов\n2. Зарегистрировать клиента\n3. Изменить контакты\n"
                     "4. Удалить клиента\n5. Все абонементы\n6. Оформить абонемент\n"
                     "7. Удалить абонемент\n0. Назад\n";
        std::string c = Console::readLine("Выберите действие: ");
        if (c == "1") run([] { printUsers(UserService::getClients()); });
        else if (c == "2") run([] { addUser(Role::Client); });
        else if (c == "3") run([] { editContacts(UserService::getClients()); });
        else if (c == "4") run([&] { deleteUser(UserService::getClients(), admin.id); });
        else if (c == "5") run([] { printSubscriptions(SubscriptionService::getAll()); });
        else if (c == "6") run([] {
            printUsers(UserService::getClients());
            int clientId = Console::readInt("ID клиента: ");
            std::string type = Console::readLine("Тип (например, Месячный): ");
            std::string start = Console::readLine("Дата начала ГГГГ-ММ-ДД [" + DateTime::today() + "]: ");
            if (start.empty()) start = DateTime::today();
            std::string end = Console::readLine("Дата окончания ГГГГ-ММ-ДД: ");
            int visits = Console::readInt("Количество посещений: ");
            Subscription s = SubscriptionService::issue(clientId, type, start, end, visits);
            Console::success("Абонемент оформлен, ID " + std::to_string(s.id));
        });
        else if (c == "7") run([] {
            printSubscriptions(SubscriptionService::getAll());
            int id = Console::readInt("ID абонемента: ");
            if (Console::confirm("Удалить абонемент?")) {
                SubscriptionService::remove(id);
                Console::success("Абонемент удалён");
            }
        });
        else if (c == "0") return;
    }
}

void exportData(const AppConfig& config) {
    auto files = Exporter::exportLessons(config.outDir);
    Console::success("Расписание выгружено:");
    for (auto& f : files) std::cout << "  " << f << "\n";
}

// ---------------- Клиент ----------------

void cancelOwnBooking(const User& user) {
    std::vector<Booking> active;
    for (auto& b : BookingService::getForClient(user.id))
        if (b.statusId == BookingStatus::Active && b.lessonStartsAt > DateTime::now())
            active.push_back(b);
    printBookings(active);
    if (active.empty()) return;
    BookingService::cancel(Console::readInt("ID записи: "), user.id);
    Console::success("Запись отменена, посещение возвращено на абонемент");
}

}  // namespace

namespace MenuUi {

void mainMenu(const AppConfig& config) {
    while (true) {
        Console::title("Fitness Club Tracker");
        std::cout << "1. Вход\n2. Регистрация клиента\n0. Выход\n";
        std::string c = Console::readLine("Выберите действие: ");
        if (c == "1") {
            std::optional<User> user;
            run([&] { user = AuthUi::login(); });
            if (!user) continue;
            if (user->roleId == Role::Admin) adminMenu(*user, config);
            else if (user->roleId == Role::Trainer) trainerMenu(*user);
            else clientMenu(*user);
        } else if (c == "2") {
            run(AuthUi::registerClient);
        } else if (c == "0") {
            return;
        }
    }
}

void adminMenu(const User& user, const AppConfig& config) {
    while (true) {
        Console::title("Администратор (" + user.login + ")");
        std::cout << "1. Расписание занятий\n"
                     "2. Тренеры\n"
                     "3. Клиенты и абонементы\n"
                     "4. Все записи на занятия\n"
                     "5. Отменить запись клиента\n"
                     "6. Экспорт расписания в JSON/CSV/XML/YAML\n"
                     "7. Добавить администратора\n"
                     "8. Пересоздать БД с тестовыми данными\n"
                     "0. Выйти из аккаунта\n";
        std::string c = Console::readLine("Выберите действие: ");
        if (c == "1") scheduleMenu();
        else if (c == "2") trainersMenu(user);
        else if (c == "3") clientsMenu(user);
        else if (c == "4") run([] { printBookings(BookingService::getAll()); });
        else if (c == "5") run([&] {
            printBookings(BookingService::getAll());
            BookingService::cancel(Console::readInt("ID записи: "), user.id, true);
            Console::success("Запись отменена");
        });
        else if (c == "6") run([&] { exportData(config); });
        else if (c == "7") run([] { addUser(Role::Admin); });
        else if (c == "8") {
            if (Console::confirm("Все данные будут заменены тестовыми. Продолжить?")) {
                run([&] { Database::instance().reset(config.schemaPath); });
                Console::success("База пересоздана. Войдите заново.");
                return;
            }
        } else if (c == "0") return;
    }
}

void trainerMenu(const User& user) {
    while (true) {
        Console::title("Тренер (" + user.login + ")");
        std::cout << "1. Мои занятия\n2. Записавшиеся на моё занятие\n3. Общее расписание\n"
                     "0. Выйти из аккаунта\n";
        std::string c = Console::readLine("Выберите действие: ");
        if (c == "1") run([&] { printLessons(LessonService::getSchedule(std::nullopt, user.id)); });
        else if (c == "2") run([&] {
            printLessons(LessonService::getSchedule(std::nullopt, user.id));
            int id = Console::readInt("ID занятия: ");
            if (LessonService::getById(id).trainerId != user.id)
                throw ServiceError("Это занятие ведёт другой тренер");
            printBookings(BookingService::getForLesson(id));
        });
        else if (c == "3") run([] { printLessons(LessonService::getSchedule()); });
        else if (c == "0") return;
    }
}

void clientMenu(const User& user) {
    while (true) {
        Console::title("Клиент (" + user.login + ")");
        std::cout << "1. Расписание (предстоящие занятия)\n"
                     "2. Фильтр расписания по дате и тренеру\n"
                     "3. Записаться на занятие\n"
                     "4. Мои записи\n"
                     "5. Отменить запись\n"
                     "6. Мой абонемент\n"
                     "0. Выйти из аккаунта\n";
        std::string c = Console::readLine("Выберите действие: ");
        if (c == "1") run([] { printLessons(LessonService::getUpcoming()); });
        else if (c == "2") run([] { filterSchedule(true); });
        else if (c == "3") run([&] {
            printLessons(LessonService::getUpcoming());
            Booking b = BookingService::book(user.id, Console::readInt("ID занятия: "));
            Console::success("Вы записаны на «" + b.lessonTitle + "» " + b.lessonStartsAt);
        });
        else if (c == "4") run([&] { printBookings(BookingService::getForClient(user.id)); });
        else if (c == "5") run([&] { cancelOwnBooking(user); });
        else if (c == "6") run([&] { printSubscriptions(SubscriptionService::getForClient(user.id)); });
        else if (c == "0") return;
    }
}

}  // namespace MenuUi
