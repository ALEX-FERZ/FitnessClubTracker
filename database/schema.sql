-- =====================================================================
--  Fitness Club Tracker — схема базы данных SQLite (ЛР №3)
--  Порядок: удаление зависимых таблиц -> создание справочников ->
--           основных таблиц -> таблиц связей -> тестовые данные.
-- =====================================================================

PRAGMA foreign_keys = ON;

DROP TABLE IF EXISTS bookings;
DROP TABLE IF EXISTS subscriptions;
DROP TABLE IF EXISTS lessons;
DROP TABLE IF EXISTS users;
DROP TABLE IF EXISTS booking_statuses;
DROP TABLE IF EXISTS roles;

-- ---------- Справочники ----------

CREATE TABLE roles (
    role_id INTEGER PRIMARY KEY AUTOINCREMENT,
    name    TEXT NOT NULL UNIQUE
);

CREATE TABLE booking_statuses (
    status_id INTEGER PRIMARY KEY AUTOINCREMENT,
    name      TEXT NOT NULL UNIQUE
);

-- ---------- Основные таблицы ----------

-- Пользователи системы: клиенты, тренеры и администраторы
CREATE TABLE users (
    user_id       INTEGER PRIMARY KEY AUTOINCREMENT,
    login         TEXT NOT NULL UNIQUE,
    password_hash TEXT NOT NULL,
    role_id       INTEGER NOT NULL,
    full_name     TEXT NOT NULL,
    phone         TEXT,
    FOREIGN KEY (role_id) REFERENCES roles (role_id)
);

-- Занятия в расписании (каждое ведёт один тренер)
CREATE TABLE lessons (
    lesson_id  INTEGER PRIMARY KEY AUTOINCREMENT,
    trainer_id INTEGER NOT NULL,
    title      TEXT NOT NULL,
    starts_at  TEXT NOT NULL,                       -- 'YYYY-MM-DD HH:MM'
    capacity   INTEGER NOT NULL CHECK (capacity > 0),
    FOREIGN KEY (trainer_id) REFERENCES users (user_id)
);

-- Абонементы клиентов
CREATE TABLE subscriptions (
    subscription_id INTEGER PRIMARY KEY AUTOINCREMENT,
    client_id       INTEGER NOT NULL,
    type            TEXT NOT NULL,
    start_date      TEXT NOT NULL,                  -- 'YYYY-MM-DD'
    end_date        TEXT NOT NULL,                  -- 'YYYY-MM-DD'
    visits_left     INTEGER NOT NULL DEFAULT 0 CHECK (visits_left >= 0),
    FOREIGN KEY (client_id) REFERENCES users (user_id) ON DELETE CASCADE
);

-- ---------- Таблица связи (клиенты M:N занятия) ----------

CREATE TABLE bookings (
    booking_id INTEGER PRIMARY KEY AUTOINCREMENT,
    client_id  INTEGER NOT NULL,
    lesson_id  INTEGER NOT NULL,
    created_at TEXT NOT NULL DEFAULT (strftime('%Y-%m-%d %H:%M', 'now', 'localtime')),
    status_id  INTEGER NOT NULL DEFAULT 1,
    FOREIGN KEY (client_id) REFERENCES users (user_id) ON DELETE CASCADE,
    FOREIGN KEY (lesson_id) REFERENCES lessons (lesson_id) ON DELETE CASCADE,
    FOREIGN KEY (status_id) REFERENCES booking_statuses (status_id)
);

-- ---------- Тестовые данные ----------

INSERT INTO roles (name) VALUES
    ('Клиент'),          -- 1
    ('Тренер'),          -- 2
    ('Администратор');   -- 3

INSERT INTO booking_statuses (name) VALUES
    ('Активна'),         -- 1
    ('Отменена');        -- 2

-- Пароли тестовых пользователей (хэш FNV-1a от "login:password", см. AuthService):
--   admin / admin123, sidorov и kuznetsova / trainer123,
--   ivanov, petrova, smirnov / client123
INSERT INTO users (login, password_hash, role_id, full_name, phone) VALUES
    ('admin',      'a4239af2c06b4307', 3, 'Орлова Екатерина Андреевна', '+79000000001'),
    ('sidorov',    '528da99a3a4dae3c', 2, 'Сидоров Алексей Петрович',  '+79003334455'),
    ('kuznetsova', '22b4a0b4f9550694', 2, 'Кузнецова Мария Олеговна',  '+79004445566'),
    ('ivanov',     'bb6891fa063acfa7', 1, 'Иванов Иван Иванович',      '+79001112233'),
    ('petrova',    'cfae0447a0c3e9df', 1, 'Петрова Анна Сергеевна',    '+79002223344'),
    ('smirnov',    'e878b776066e110a', 1, 'Смирнов Олег Дмитриевич',   NULL);

INSERT INTO lessons (trainer_id, title, starts_at, capacity) VALUES
    (2, 'Силовая тренировка',  '2026-11-20 18:00', 10),
    (2, 'Кроссфит',            '2026-11-21 19:00', 2),
    (3, 'Йога для начинающих', '2026-11-22 10:00', 15),
    (3, 'Пилатес',             '2026-12-05 11:00', 12),
    (2, 'Функциональный тренинг', '2026-12-10 18:30', 8);

INSERT INTO subscriptions (client_id, type, start_date, end_date, visits_left) VALUES
    (4, 'Месячный',          '2026-11-01', '2026-11-30', 7),
    (5, 'Разовые посещения', '2026-11-01', '2027-01-31', 3),
    (6, 'Годовой',           '2026-09-01', '2027-08-31', 100);

INSERT INTO bookings (client_id, lesson_id, created_at, status_id) VALUES
    (4, 1, '2026-11-01 12:00', 1),
    (4, 3, '2026-11-01 12:05', 2),
    (5, 2, '2026-11-02 13:30', 1),
    (6, 2, '2026-11-02 14:10', 1),
    (6, 3, '2026-11-03 09:00', 1);
