#pragma once
// Репозитории (Data Access Layer). SQL-запросы находятся ТОЛЬКО здесь.
// Каждый namespace работает с одной таблицей и реализует операции CRUD.

#include <optional>
#include <string>
#include <vector>

#include "models/Models.h"

namespace DictionaryRepository {  // roles, booking_statuses (только чтение)
std::vector<RoleInfo> getRoles();
std::vector<BookingStatusInfo> getBookingStatuses();
}  // namespace DictionaryRepository

namespace UserRepository {
int create(const std::string& login, const std::string& passwordHash, int roleId,
           const std::string& fullName, const std::optional<std::string>& phone);
std::optional<User> getById(int id);
std::optional<User> getByLogin(const std::string& login);
std::vector<User> getAll();
std::vector<User> getByRole(int roleId);
void update(const User& user);
void remove(int id);
}  // namespace UserRepository

namespace LessonRepository {
int create(int trainerId, const std::string& title, const std::string& startsAt, int capacity);
std::optional<Lesson> getById(int id);
std::vector<Lesson> getAll();
std::vector<Lesson> getByTrainer(int trainerId);
void update(const Lesson& lesson);
void remove(int id);
}  // namespace LessonRepository

namespace SubscriptionRepository {
int create(int clientId, const std::string& type, const std::string& startDate,
           const std::string& endDate, int visitsLeft);
std::optional<Subscription> getById(int id);
std::vector<Subscription> getAll();
std::vector<Subscription> getByClient(int clientId);
void updateVisits(int id, int visitsLeft);
void remove(int id);
}  // namespace SubscriptionRepository

namespace BookingRepository {
int create(int clientId, int lessonId);
std::optional<Booking> getById(int id);
std::vector<Booking> getAll();
std::vector<Booking> getByClient(int clientId);
std::vector<Booking> getByLesson(int lessonId);
std::optional<Booking> findActive(int clientId, int lessonId);
int countActiveByLesson(int lessonId);
void updateStatus(int id, int statusId);
void remove(int id);
}  // namespace BookingRepository
