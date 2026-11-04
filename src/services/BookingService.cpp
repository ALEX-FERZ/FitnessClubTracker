#include "repositories/Repositories.h"
#include "services/Services.h"

namespace SubscriptionService {

std::vector<Subscription> getAll() { return SubscriptionRepository::getAll(); }

std::vector<Subscription> getForClient(int clientId) {
    return SubscriptionRepository::getByClient(clientId);
}

std::optional<Subscription> findValid(int clientId, const std::string& date) {
    for (auto& s : SubscriptionRepository::getByClient(clientId))
        if (s.startDate <= date && date <= s.endDate && s.visitsLeft > 0) return s;
    return std::nullopt;
}

Subscription issue(int clientId, const std::string& type, const std::string& startDate,
                   const std::string& endDate, int visits) {
    auto client = UserRepository::getById(clientId);
    if (!client || client->roleId != Role::Client) throw ServiceError("Клиент не найден");
    if (type.find_first_not_of(' ') == std::string::npos)
        throw ServiceError("Тип абонемента не может быть пустым");
    if (!DateTime::isValidDate(startDate) || !DateTime::isValidDate(endDate))
        throw ServiceError("Даты должны быть в формате ГГГГ-ММ-ДД");
    if (endDate < startDate) throw ServiceError("Дата окончания раньше даты начала");
    if (endDate < DateTime::today()) throw ServiceError("Срок абонемента уже истёк");
    if (visits < 1 || visits > 365) throw ServiceError("Число посещений должно быть от 1 до 365");
    int id = SubscriptionRepository::create(clientId, type, startDate, endDate, visits);
    return *SubscriptionRepository::getById(id);
}

void remove(int subscriptionId) {
    if (!SubscriptionRepository::getById(subscriptionId)) throw ServiceError("Абонемент не найден");
    SubscriptionRepository::remove(subscriptionId);
}

}  // namespace SubscriptionService

namespace BookingService {

std::vector<Booking> getAll() { return BookingRepository::getAll(); }
std::vector<Booking> getForClient(int clientId) { return BookingRepository::getByClient(clientId); }
std::vector<Booking> getForLesson(int lessonId) { return BookingRepository::getByLesson(lessonId); }

// Алгоритм записи на занятие (см. docs/diagrams/flow_booking.puml).
Booking book(int clientId, int lessonId) {
    auto client = UserRepository::getById(clientId);
    if (!client || client->roleId != Role::Client)
        throw ServiceError("Записываться на занятия может только клиент");

    auto lesson = LessonRepository::getById(lessonId);
    if (!lesson) throw ServiceError("Занятие не найдено");
    if (lesson->startsAt <= DateTime::now()) throw ServiceError("Занятие уже прошло");

    if (BookingRepository::findActive(clientId, lessonId))
        throw ServiceError("Вы уже записаны на это занятие");

    if (BookingRepository::countActiveByLesson(lessonId) >= lesson->capacity)
        throw ServiceError("Свободных мест нет");

    auto sub = SubscriptionService::findValid(clientId, lesson->startsAt.substr(0, 10));
    if (!sub)
        throw ServiceError("Нет действующего абонемента на дату занятия (или закончились посещения)");

    int bookingId = BookingRepository::create(clientId, lessonId);
    SubscriptionRepository::updateVisits(sub->id, sub->visitsLeft - 1);
    return *BookingRepository::getById(bookingId);
}

void cancel(int bookingId, int userId, bool byAdmin) {
    auto booking = BookingRepository::getById(bookingId);
    if (!booking) throw ServiceError("Запись не найдена");
    if (!byAdmin && booking->clientId != userId) throw ServiceError("Это не ваша запись");
    if (booking->statusId != BookingStatus::Active) throw ServiceError("Запись уже отменена");

    long long minutesLeft =
        DateTime::toMinutes(booking->lessonStartsAt) - DateTime::toMinutes(DateTime::now());
    if (minutesLeft <= 0) throw ServiceError("Занятие уже началось или прошло");
    if (!byAdmin && minutesLeft < kCancelDeadlineMinutes)
        throw ServiceError("Отменить запись можно не позднее чем за 2 часа до начала");

    BookingRepository::updateStatus(bookingId, BookingStatus::Cancelled);

    // Возвращаем посещение на абонемент, действующий на дату занятия.
    std::string date = booking->lessonStartsAt.substr(0, 10);
    for (auto& s : SubscriptionRepository::getByClient(booking->clientId)) {
        if (s.startDate <= date && date <= s.endDate) {
            SubscriptionRepository::updateVisits(s.id, s.visitsLeft + 1);
            break;
        }
    }
}

}  // namespace BookingService
