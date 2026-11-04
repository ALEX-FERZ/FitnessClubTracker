#pragma once
#include <optional>
#include <vector>
#include "models/Booking.h"

namespace BookingRepository {
int create(int clientId, int lessonId);
std::optional<Booking> getById(int id);
std::vector<Booking> getByClient(int clientId);
std::vector<Booking> getByLesson(int lessonId);
void updateStatus(int id, int status);
}
