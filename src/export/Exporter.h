#pragma once
#include <string>

// Экспорт данных в JSON, CSV, XML и YAML (ЛР №5).
//
// Код универсальный: структура задаётся названиями столбцов в SQL-запросе.
//   "trainer.fullName" — поле fullName вложенного объекта trainer
//   "clients[]"        — список (элементы в строке через ';', например group_concat)
// Для другой таблицы достаточно передать другой запрос.
namespace Exporter {
void exportQuery(const std::string& sql, const std::string& tableName, const std::string& outDir);

// Занятия + тренер (вложенный объект) + записавшиеся клиенты (список)
void exportLessons(const std::string& outDir);
}
