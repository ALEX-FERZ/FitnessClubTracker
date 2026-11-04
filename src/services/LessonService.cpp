#include "services/LessonService.h"
#include <stdexcept>
#include "repositories/BookingRepository.h"
#include "repositories/LessonRepository.h"
#include "repositories/UserRepository.h"

namespace {
// Проверка данных занятия: название, дата вида 2026-10-20 18:00, количество мест
void check(const std::string& title, const std::string& startsAt, int capacity) {
    if (title.empty()) throw std::runtime_error("Название не может быть пустым");
    if (startsAt.size() < 16 || startsAt[4] != '-' || startsAt[7] != '-' || startsAt[13] != ':')
        throw std::runtime_error("Дата должна быть в формате ГГГГ-ММ-ДД ЧЧ:ММ");
    if (capacity <= 0) throw std::runtime_error("Количество мест должно быть больше 0");
}
}

namespace LessonService {

std::vector<Lesson> getSchedule() { return LessonRepository::getAll(); }

Lesson getById(int lessonId) {
    auto lesson = LessonRepository::getById(lessonId);
    if (!lesson) throw std::runtime_error("Занятие не найдено");
    return *lesson;
}

void addLesson(int trainerId, const std::string& title, const std::string& startsAt, int capacity) {
    auto trainer = UserRepository::getById(trainerId);
    if (!trainer || trainer->role != ROLE_TRAINER) throw std::runtime_error("Тренер не найден");
    check(title, startsAt, capacity);

    // один тренер не может вести два занятия в одно и то же время
    for (const Lesson& l : LessonRepository::getAll())
        if (l.trainerId == trainerId && l.startsAt.substr(0, 16) == startsAt.substr(0, 16))
            throw std::runtime_error("У тренера уже есть занятие в это время");

    LessonRepository::create(Lesson{0, trainerId, title, startsAt, capacity});
}

void editLesson(int lessonId, const std::string& title, const std::string& startsAt, int capacity) {
    Lesson lesson = getById(lessonId);
    check(title, startsAt, capacity);
    if (capacity < lesson.booked)
        throw std::runtime_error("Мест меньше, чем уже записано клиентов");
    lesson.title = title;
    lesson.startsAt = startsAt;
    lesson.capacity = capacity;
    LessonRepository::update(lesson);
}

void deleteLesson(int lessonId) {
    getById(lessonId);  // проверка, что занятие существует
    LessonRepository::remove(lessonId);
}

std::vector<Booking> getBookings(int lessonId) {
    getById(lessonId);
    return BookingRepository::getByLesson(lessonId);
}

}
