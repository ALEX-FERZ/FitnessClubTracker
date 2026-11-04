#include <cstdlib>

#include "repositories/Repositories.h"
#include "services/Services.h"

namespace {
void validateLesson(int lessonId, int trainerId, const std::string& title,
                    const std::string& startsAt, int capacity) {
    auto trainer = UserRepository::getById(trainerId);
    if (!trainer || trainer->roleId != Role::Trainer) throw ServiceError("Тренер не найден");
    if (title.find_first_not_of(' ') == std::string::npos)
        throw ServiceError("Название занятия не может быть пустым");
    if (!DateTime::isValidDateTime(startsAt))
        throw ServiceError("Дата и время должны быть в формате ГГГГ-ММ-ДД ЧЧ:ММ");
    if (startsAt < DateTime::now()) throw ServiceError("Нельзя назначить занятие на прошедшее время");
    if (capacity < 1 || capacity > 100) throw ServiceError("Вместимость должна быть от 1 до 100");

    // Контроль накладок: тренер не может вести два занятия одновременно.
    long long start = DateTime::toMinutes(startsAt);
    for (const auto& other : LessonRepository::getByTrainer(trainerId)) {
        if (other.id == lessonId) continue;
        long long diff = std::llabs(DateTime::toMinutes(other.startsAt) - start);
        if (diff < LessonService::kLessonDurationMinutes)
            throw ServiceError("Накладка в расписании: у тренера уже есть занятие «" + other.title +
                               "» в " + other.startsAt);
    }
}
}  // namespace

namespace LessonService {

std::vector<Lesson> getSchedule(const std::optional<std::string>& date,
                                const std::optional<int>& trainerId) {
    if (date && !DateTime::isValidDate(*date))
        throw ServiceError("Дата должна быть в формате ГГГГ-ММ-ДД");
    std::vector<Lesson> result;
    for (auto& l : LessonRepository::getAll()) {
        if (date && l.startsAt.compare(0, 10, *date) != 0) continue;
        if (trainerId && l.trainerId != *trainerId) continue;
        result.push_back(l);
    }
    return result;
}

std::vector<Lesson> getUpcoming() {
    std::vector<Lesson> result;
    std::string now = DateTime::now();
    for (auto& l : LessonRepository::getAll())
        if (l.startsAt > now) result.push_back(l);
    return result;
}

Lesson getById(int lessonId) {
    auto lesson = LessonRepository::getById(lessonId);
    if (!lesson) throw ServiceError("Занятие не найдено");
    return *lesson;
}

Lesson createLesson(int trainerId, const std::string& title, const std::string& startsAt,
                    int capacity) {
    validateLesson(0, trainerId, title, startsAt, capacity);
    return getById(LessonRepository::create(trainerId, title, startsAt, capacity));
}

void updateLesson(int lessonId, int trainerId, const std::string& title,
                  const std::string& startsAt, int capacity) {
    Lesson lesson = getById(lessonId);
    validateLesson(lessonId, trainerId, title, startsAt, capacity);
    if (capacity < lesson.bookedCount)
        throw ServiceError("Вместимость меньше числа уже записавшихся (" +
                           std::to_string(lesson.bookedCount) + ")");
    lesson.trainerId = trainerId;
    lesson.title = title;
    lesson.startsAt = startsAt;
    lesson.capacity = capacity;
    LessonRepository::update(lesson);
}

void deleteLesson(int lessonId) {
    Lesson lesson = getById(lessonId);
    // Клиентам с активной записью на будущее занятие возвращаем посещение.
    if (lesson.startsAt > DateTime::now()) {
        std::string date = lesson.startsAt.substr(0, 10);
        for (const auto& b : BookingRepository::getByLesson(lessonId)) {
            if (b.statusId != BookingStatus::Active) continue;
            for (const auto& s : SubscriptionRepository::getByClient(b.clientId)) {
                if (s.startDate <= date && date <= s.endDate) {
                    SubscriptionRepository::updateVisits(s.id, s.visitsLeft + 1);
                    break;
                }
            }
        }
    }
    LessonRepository::remove(lessonId);  // записи удаляются каскадно
}

}  // namespace LessonService
