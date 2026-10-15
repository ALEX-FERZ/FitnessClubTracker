DROP TABLE IF EXISTS Booking;
DROP TABLE IF EXISTS Subscription;
DROP TABLE IF EXISTS Lesson;
DROP TABLE IF EXISTS User;

-- role: 0 - клиент, 1 - тренер
CREATE TABLE User (
UserID INTEGER PRIMARY KEY AUTOINCREMENT,
login TEXT NOT NULL UNIQUE,
passwordHash TEXT NOT NULL,
role INTEGER NOT NULL DEFAULT 0 CHECK (role IN (0, 1)),
fullName TEXT NOT NULL,
phone TEXT
);

CREATE TABLE Lesson (
LessonID INTEGER PRIMARY KEY AUTOINCREMENT,
TrainerID INTEGER NOT NULL,
title TEXT NOT NULL,
startTime TEXT NOT NULL,
maxPlaces INTEGER NOT NULL CHECK (maxPlaces > 0),
FOREIGN KEY (TrainerID) REFERENCES User(UserID)
);

CREATE TABLE Subscription (
SubscriptionID INTEGER PRIMARY KEY AUTOINCREMENT,
ClientID INTEGER NOT NULL,
type TEXT NOT NULL,
startDate TEXT NOT NULL,
endDate TEXT NOT NULL,
visitsLeft INTEGER NOT NULL DEFAULT 0 CHECK (visitsLeft >= 0),
FOREIGN KEY (ClientID) REFERENCES User(UserID)
);

-- status: 0 - отменена, 1 - активна
CREATE TABLE Booking (
BookingID INTEGER PRIMARY KEY AUTOINCREMENT,
ClientID INTEGER NOT NULL,
LessonID INTEGER NOT NULL,
createdAt TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
status INTEGER NOT NULL DEFAULT 1 CHECK (status IN (0, 1)),
FOREIGN KEY (ClientID) REFERENCES User(UserID),
FOREIGN KEY (LessonID) REFERENCES Lesson(LessonID)
);


INSERT INTO User (login, passwordHash, role, fullName, phone) VALUES
('ivanov', 'hash_1', 0, 'Иванов Иван Иванович', '+79001112233'), 
('petrova', 'hash_2', 0, 'Петрова Анна Сергеевна', '+79002223344'), 
('sidorov', 'hash_3', 1, 'Сидоров Алексей Петрович', '+79003334455'), 
('kuznetsova', 'hash_4', 1, 'Кузнецова Мария Олеговна', '+79004445566'); 

INSERT INTO Lesson (TrainerID, title, startTime, maxPlaces) VALUES
(3, 'Силовая тренировка', '2026-10-20 18:00:00', 10),
(3, 'Кроссфит', '2026-10-21 19:00:00', 8),
(4, 'Йога для начинающих', '2026-10-22 10:00:00', 15);

INSERT INTO Subscription (ClientID, type, startDate, endDate, visitsLeft) VALUES
(1, 'Месячный', '2026-10-01', '2026-10-31', 8),
(2, 'Разовые посещения', '2026-10-05', '2026-12-05', 4);

INSERT INTO Booking (ClientID, LessonID, createdAt, status) VALUES
(1, 1, '2026-10-15 12:00:00', 1),
(1, 3, '2026-10-15 12:05:00', 0),
(2, 2, '2026-10-15 13:30:00', 1);
