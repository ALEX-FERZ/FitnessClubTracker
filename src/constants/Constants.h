#pragma once
#include <cstddef>
#include <string>

// Константы проекта. Здесь только объявления (extern),
// сами значения задаются один раз в Constants.cpp.

// Роли пользователей (поле role таблицы User)
extern const int ROLE_CLIENT;
extern const int ROLE_ADMIN;

// Статусы записи на занятие (поле status таблицы Booking)
extern const int BOOKING_CANCELLED;
extern const int BOOKING_ACTIVE;

// Пути к файлам (относительно корня проекта)
extern const std::string DB_PATH;
extern const std::string SQL_PATH;
extern const std::string OUT_DIR;

// Пароли
extern const std::size_t MIN_PASSWORD_LENGTH;
extern const std::string PASSWORD_PREFIX;
