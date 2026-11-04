#include <iostream>
#include "database/Database.h"
#include "ui/Input.h"
#include "ui/Menu.h"

// Запускать из корневой папки проекта:  build\fitness_club.exe
int main() {
    setupConsole();

    if (!openDatabase("database/database.db", "database/database.sql")) return 1;

    mainMenu();

    closeDatabase();
    std::cout << "До свидания!\n";
    return 0;
}
