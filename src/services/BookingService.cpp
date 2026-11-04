#include "services/BookingService.h"
#include <stdexcept>
#include "repositories/BookingRepository.h"
#include "repositories/LessonRepository.h"
#include "repositories/SubscriptionRepository.h"
#include "repositories/UserRepository.h"

namespace {
// Абонемент клиента, который действует в день занятия (date — ГГГГ-ММ-ДД)
Subscription* findSubscription(std::vector<Subscription>& subs, const std::string& date) {
    for (Subscription& s : subs)
        if (s.startDate <= date && date <= s.endDate) return &s;
    return nullptr;
}
}

namespace BookingService {

void book(int clientId, int lessonId) {
    auto lesson = LessonRepository::getById(lessonId);
    if (!lesson) throw std::runtime_error("Занятие не найдено");

    for (const Booking& b : BookingRepository::getByClient(clientId))
        if (b.lessonId == lessonId && b.status == BOOKING_ACTIVE)
            throw std::runtime_error("Вы уже записаны на это занятие");

    if (lesson->booked >= lesson->capacity) throw std::runtime_error("Свободных мест нет");

    auto subs = SubscriptionRepository::getByClient(clientId);
    Subscription* sub = findSubscription(subs, lesson->startsAt.substr(0, 10));
    if (!sub) throw std::runtime_error("Нет абонемента, действующего в день занятия");
    if (sub->visitsLeft <= 0) throw std::runtime_error("На абонементе закончились посещения");

    BookingRepository::create(clientId, lessonId);
    SubscriptionRepository::updateVisits(sub->id, sub->visitsLeft - 1);
}

std::vector<Booking> getClientBookings(int clientId) { return BookingRepository::getByClient(clientId); }

std::vector<Subscription> getSubscriptions(int clientId) { return SubscriptionRepository::getByClient(clientId); }

void addSubscription(int clientId, const std::string& type, const std::string& startDate,
                     const std::string& endDate, int visits) {
    auto client = UserRepository::getById(clientId);
    if (!client || client->role != ROLE_CLIENT) throw std::runtime_error("Клиент не найден");
    if (type.empty()) throw std::runtime_error("Тип абонемента не может быть пустым");
    if (startDate.size() != 10 || endDate.size() != 10)
        throw std::runtime_error("Даты должны быть в формате ГГГГ-ММ-ДД");
    if (endDate < startDate) throw std::runtime_error("Дата окончания раньше даты начала");
    if (visits <= 0) throw std::runtime_error("Количество посещений должно быть больше 0");

    SubscriptionRepository::create(Subscription{0, clientId, type, startDate, endDate, visits});
}

}
