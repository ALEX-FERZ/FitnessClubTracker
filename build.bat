@echo off
rem Сборка программы. Запускать из корня проекта, результат: build\fitness_club.exe
chcp 65001 >nul
if not exist build mkdir build

rem SQLite компилируется один раз (долго), потом используется готовый файл
if not exist build\sqlite3.o gcc -c -O2 third_party\sqlite\sqlite3.c -o build\sqlite3.o

g++ -std=c++17 -Isrc -Ithird_party\sqlite ^
    src\main.cpp src\database\*.cpp src\repositories\*.cpp src\services\*.cpp ^
    src\ui\*.cpp src\export\*.cpp build\sqlite3.o -static -o build\fitness_club.exe

if %errorlevel% == 0 echo Готово: build\fitness_club.exe
