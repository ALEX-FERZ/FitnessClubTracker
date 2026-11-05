#include "constants/Constants.h"

// role: 0 - клиент, 1 - администратор
const int ROLE_CLIENT = 0;
const int ROLE_ADMIN = 1;

// status: 0 - отменена, 1 - активна
const int BOOKING_CANCELLED = 0;
const int BOOKING_ACTIVE = 1;

const std::string DB_PATH = "database/database.db";
const std::string SQL_PATH = "database/database.sql";
const std::string OUT_DIR = "out";

const std::size_t MIN_PASSWORD_LENGTH = 4;
// Учебный «хэш»: пароль хранится как hash_<пароль>
const std::string PASSWORD_PREFIX = "hash_";
