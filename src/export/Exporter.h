#pragma once
// Экспорт результата SQL-запроса в JSON, CSV, XML и YAML (ЛР №5).
//
// Универсальность: структура вложенности задаётся ПСЕВДОНИМАМИ столбцов в SQL,
// поэтому для другой таблицы достаточно поменять запрос:
//   "trainer.full_name"  -> вложенный объект  trainer: { full_name: ... }
//   "clients[]"          -> массив; значение столбца — строка с элементами через ';'
//                           (удобно получать через group_concat(..., ';'))
// Типы значений берутся из SQLite (INTEGER, REAL, TEXT, NULL).

#include <string>
#include <vector>

#include "export/DataNode.h"

namespace Serializer {
std::string toJson(const DataNode& node);
// rows — массив объектов; вложенные объекты разворачиваются в столбцы a.b,
// массивы склеиваются через ';'.
std::string toCsv(const DataNode& rows);
std::string toXml(const DataNode& node, const std::string& rootTag);
std::string toYaml(const DataNode& node);
}  // namespace Serializer

namespace Exporter {
// Выполняет SELECT и строит массив объектов по правилам выше.
DataNode queryToNodes(const std::string& sql);

// Создаёт папку outDir (если её нет) и записывает data.json, data.csv, data.xml,
// data.yaml. Возвращает список созданных файлов.
std::vector<std::string> exportQuery(const std::string& sql, const std::string& collection,
                                     const std::string& outDir);

// Экспорт расписания: занятия + вложенный тренер + список записавшихся клиентов.
std::vector<std::string> exportLessons(const std::string& outDir);
}  // namespace Exporter
