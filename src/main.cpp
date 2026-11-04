// Fitness Club Tracker — точка входа.
//
// Запуск (из корня проекта):
//   build\fitness_club.exe            — консольное меню
//   build\fitness_club.exe --export   — только выгрузка расписания в папку out (ЛР №5)
//   build\fitness_club.exe --reset    — пересоздать БД из database/schema.sql

#include <iostream>
#include <string>

#include "database/Database.h"
#include "export/Exporter.h"
#include "ui/Console.h"
#include "ui/Menu.h"

int main(int argc, char* argv[]) {
    Console::setup();
    AppConfig config;

    try {
        Database::instance().open(config.dbPath, config.schemaPath);

        std::string arg = argc > 1 ? argv[1] : "";
        if (arg == "--reset") {
            Database::instance().reset(config.schemaPath);
            std::cout << "БД пересоздана: " << config.dbPath << "\n";
            return 0;
        }
        if (arg == "--export") {
            for (auto& file : Exporter::exportLessons(config.outDir)) std::cout << file << "\n";
            return 0;
        }

        MenuUi::mainMenu(config);
    } catch (const Console::InputClosed&) {
        std::cout << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Критическая ошибка: " << e.what() << "\n"
                  << "Запускайте программу из корневой папки проекта.\n";
        return 1;
    }
    std::cout << "До свидания!\n";
    return 0;
}
