@echo off
rem Сборка Fitness Club Tracker (MinGW-w64 g++ / MSYS2 UCRT64).
rem Запуск из корня проекта:  build.bat   ->  build\fitness_club.exe
chcp 65001 >nul
if not exist build mkdir build

rem SQLite компилируется один раз (это долго), дальше берётся готовый объектный файл.
if not exist build\sqlite3.o (
    echo Компиляция SQLite...
    gcc -c -O2 -DSQLITE_OMIT_LOAD_EXTENSION third_party\sqlite\sqlite3.c -o build\sqlite3.o || exit /b 1
)

echo Компиляция программы...
g++ -std=c++17 -O2 -Wall -Wextra -Isrc -Ithird_party\sqlite ^
    src\main.cpp ^
    src\database\Database.cpp ^
    src\repositories\DictionaryRepository.cpp ^
    src\repositories\UserRepository.cpp ^
    src\repositories\LessonRepository.cpp ^
    src\repositories\SubscriptionRepository.cpp ^
    src\repositories\BookingRepository.cpp ^
    src\services\DateTime.cpp ^
    src\services\AuthService.cpp ^
    src\services\LessonService.cpp ^
    src\services\BookingService.cpp ^
    src\export\Serializers.cpp ^
    src\export\Exporter.cpp ^
    src\ui\Console.cpp ^
    src\ui\Auth.cpp ^
    src\ui\Menu.cpp ^
    build\sqlite3.o -static -o build\fitness_club.exe || exit /b 1

echo Готово: build\fitness_club.exe
