#pragma once
#include <optional>
#include <vector>
#include "models/Lesson.h"

namespace LessonRepository {
int create(const Lesson& lesson);
std::optional<Lesson> getById(int id);
std::vector<Lesson> getAll();
void update(const Lesson& lesson);
void remove(int id);
}
