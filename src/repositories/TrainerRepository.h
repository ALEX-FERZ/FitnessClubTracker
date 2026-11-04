#pragma once
#include <optional>
#include <vector>
#include "models/Trainer.h"

namespace TrainerRepository {
int create(const Trainer& trainer);
std::optional<Trainer> getById(int id);
std::vector<Trainer> getAll();
}
