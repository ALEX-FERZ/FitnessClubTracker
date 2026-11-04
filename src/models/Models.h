#pragma once
// Модели данных: каждая структура соответствует таблице БД.
// Поля после комментария «служебные» в таблицах не хранятся — они заполняются
// при JOIN-запросах для удобного отображения в интерфейсе.

#include <optional>
#include <string>

namespace Role {
constexpr int Client = 1;
constexpr int Trainer = 2;
constexpr int Admin = 3;
}  // namespace Role

namespace BookingStatus {
constexpr int Active = 1;
constexpr int Cancelled = 2;
}  // namespace BookingStatus

struct RoleInfo {  // roles
    int id = 0;
    std::string name;
};

struct BookingStatusInfo {  // booking_statuses
    int id = 0;
    std::string name;
};

struct User {  // users
    int id = 0;
    std::string login;
    std::string passwordHash;
    int roleId = Role::Client;
    std::string fullName;
    std::optional<std::string> phone;
    // служебные
    std::string roleName;
};

struct Lesson {  // lessons
    int id = 0;
    int trainerId = 0;
    std::string title;
    std::string startsAt;  // YYYY-MM-DD HH:MM
    int capacity = 0;
    // служебные
    std::string trainerName;
    int bookedCount = 0;
    int freePlaces() const { return capacity - bookedCount; }
};

struct Subscription {  // subscriptions
    int id = 0;
    int clientId = 0;
    std::string type;
    std::string startDate;  // YYYY-MM-DD
    std::string endDate;    // YYYY-MM-DD
    int visitsLeft = 0;
    // служебные
    std::string clientName;
};

struct Booking {  // bookings
    int id = 0;
    int clientId = 0;
    int lessonId = 0;
    std::string createdAt;
    int statusId = BookingStatus::Active;
    // служебные
    std::string statusName;
    std::string clientName;
    std::string lessonTitle;
    std::string lessonStartsAt;
};
