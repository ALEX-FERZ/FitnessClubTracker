#pragma once
#include <string>
#include <vector>
#include "models/Booking.h"
#include "models/Lesson.h"
#include "models/Trainer.h"

namespace LessonService {
std::vector<Lesson> getSchedule();
std::vector<Trainer> getTrainers();
void addTrainer(const std::string& fullName, const std::string& phone);
Lesson getById(int lessonId);
void addLesson(int trainerId, const std::string& title, const std::string& startsAt, int capacity);
void editLesson(int lessonId, const std::string& title, const std::string& startsAt, int capacity);
void deleteLesson(int lessonId);
std::vector<Booking> getBookings(int lessonId);
}
