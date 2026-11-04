#pragma once
#include <vector>
#include "models/Booking.h"

namespace BookingRepository {
int create(int clientId, int lessonId);
std::vector<Booking> getByClient(int clientId);
std::vector<Booking> getByLesson(int lessonId);
}
