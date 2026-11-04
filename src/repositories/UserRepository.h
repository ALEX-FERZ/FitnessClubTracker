#pragma once
#include <optional>
#include <vector>
#include "models/User.h"

namespace UserRepository {
int create(const User& user);
std::optional<User> getById(int id);
std::optional<User> getByLogin(const std::string& login);
std::vector<User> getByRole(int role);
void update(const User& user);
void remove(int id);
}
