#pragma once
#include <string>
#include <vector>
#include "models/Booking.h"
#include "models/Subscription.h"

namespace BookingService {
// Запись клиента на занятие (с проверкой мест и абонемента)
void book(int clientId, int lessonId);
std::vector<Booking> getClientBookings(int clientId);

std::vector<Subscription> getSubscriptions(int clientId);
void addSubscription(int clientId, const std::string& type, const std::string& startDate,
                     const std::string& endDate, int visits);
}
