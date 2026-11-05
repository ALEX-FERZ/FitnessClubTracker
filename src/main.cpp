#include <iostream>
#include "constants/Constants.h"
#include "database/Database.h"
#include "ui/Input.h"
#include "ui/Menu.h"

// Запускать из корневой папки проекта:  build\fitness_club.exe
int main() {
    setupConsole();

    if (!openDatabase(DB_PATH, SQL_PATH)) return 1;

    mainMenu();

    closeDatabase();
    std::cout << "До свидания!\n";
    return 0;
}
