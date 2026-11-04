#pragma once
// Слой бизнес-логики (Service Layer). Здесь нет SQL — только проверки и правила,
// работа с данными идёт через репозитории. Ошибки сообщаются через ServiceError.

#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "models/Models.h"

class ServiceError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// ---- Дата и время (строки формата YYYY-MM-DD и YYYY-MM-DD HH:MM) ----
namespace DateTime {
std::string now();    // текущие дата и время, YYYY-MM-DD HH:MM
std::string today();  // текущая дата, YYYY-MM-DD
bool isValidDate(const std::string& s);
bool isValidDateTime(const std::string& s);
long long toMinutes(const std::string& dateTime);  // минуты от 1970-01-01
}  // namespace DateTime

namespace AuthService {
std::string hashPassword(const std::string& login, const std::string& password);
std::optional<User> authenticate(const std::string& login, const std::string& password);
// Самостоятельная регистрация — только клиент.
User registerClient(const std::string& login, const std::string& password,
                    const std::string& fullName, const std::string& phone);
// Регистрация администратором: тренера, клиента или администратора.
User registerUser(const std::string& login, const std::string& password, int roleId,
                  const std::string& fullName, const std::string& phone);
}  // namespace AuthService

namespace UserService {
std::vector<User> getClients();
std::vector<User> getTrainers();
User getById(int id);
void updateContacts(int userId, const std::string& fullName, const std::string& phone);
void deleteUser(int userId, int currentUserId);
}  // namespace UserService

namespace LessonService {
constexpr int kLessonDurationMinutes = 60;
// Фильтры необязательные: date — YYYY-MM-DD, trainerId — id тренера.
std::vector<Lesson> getSchedule(const std::optional<std::string>& date = std::nullopt,
                                const std::optional<int>& trainerId = std::nullopt);
std::vector<Lesson> getUpcoming();
Lesson getById(int lessonId);
Lesson createLesson(int trainerId, const std::string& title, const std::string& startsAt,
                    int capacity);
void updateLesson(int lessonId, int trainerId, const std::string& title,
                  const std::string& startsAt, int capacity);
void deleteLesson(int lessonId);
}  // namespace LessonService

namespace SubscriptionService {
std::vector<Subscription> getAll();
std::vector<Subscription> getForClient(int clientId);
// Действующий абонемент на дату (срок покрывает дату и остались посещения).
std::optional<Subscription> findValid(int clientId, const std::string& date);
Subscription issue(int clientId, const std::string& type, const std::string& startDate,
                   const std::string& endDate, int visits);
void remove(int subscriptionId);
}  // namespace SubscriptionService

namespace BookingService {
constexpr int kCancelDeadlineMinutes = 120;  // отмена не позднее чем за 2 часа
std::vector<Booking> getAll();
std::vector<Booking> getForClient(int clientId);
std::vector<Booking> getForLesson(int lessonId);
Booking book(int clientId, int lessonId);
// Отмена клиентом (проверяются владелец и срок) или администратором (byAdmin = true).
void cancel(int bookingId, int userId, bool byAdmin = false);
}  // namespace BookingService
