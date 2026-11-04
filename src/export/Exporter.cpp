#include "export/Exporter.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>

#include "database/Database.h"

namespace {

void writeFile(const std::filesystem::path& path, const std::string& content) {
    std::ofstream out(path, std::ios::binary);
    if (!out) throw std::runtime_error("Не удалось записать файл " + path.string());
    out << content;
}

DataNode columnValue(const Statement& st, int col) {
    switch (st.columnType(col)) {
        case SQLITE_INTEGER: return DataNode::integer(st.columnInt(col));
        case SQLITE_FLOAT: return DataNode::real(st.columnDouble(col));
        case SQLITE_NULL: return DataNode::null();
        default: return DataNode::text(st.columnText(col));
    }
}

}  // namespace

namespace Exporter {

DataNode queryToNodes(const std::string& sql) {
    Statement st(Database::instance().handle(), sql);
    DataNode rows = DataNode::array();
    while (st.step()) {
        DataNode row = DataNode::object();
        for (int col = 0; col < st.columnCount(); ++col) {
            std::string name = st.columnName(col);
            bool isArray = name.size() > 2 && name.compare(name.size() - 2, 2, "[]") == 0;
            if (isArray) name.resize(name.size() - 2);

            // "a.b.c" -> row[a][b][c]
            DataNode* target = &row;
            std::size_t pos;
            while ((pos = name.find('.')) != std::string::npos) {
                DataNode& child = (*target)[name.substr(0, pos)];
                if (child.type != DataNode::Type::Object) child = DataNode::object();
                target = &child;
                name = name.substr(pos + 1);
            }

            if (isArray) {
                DataNode arr = DataNode::array();
                if (!st.isNull(col)) {
                    std::string list = st.columnText(col), item;
                    std::size_t start = 0, sep;
                    do {
                        sep = list.find(';', start);
                        item = list.substr(start, sep == std::string::npos ? std::string::npos
                                                                           : sep - start);
                        if (!item.empty()) arr.items.push_back(DataNode::text(item));
                        start = sep + 1;
                    } while (sep != std::string::npos);
                }
                (*target)[name] = arr;
            } else {
                (*target)[name] = columnValue(st, col);
            }
        }
        rows.items.push_back(row);
    }
    return rows;
}

std::vector<std::string> exportQuery(const std::string& sql, const std::string& collection,
                                     const std::string& outDir) {
    namespace fs = std::filesystem;
    fs::create_directories(outDir);  // создаём папку out, если её нет

    DataNode rows = queryToNodes(sql);
    DataNode root = DataNode::object();
    root[collection] = rows;

    std::vector<std::pair<std::string, std::string>> files = {
        {"data.json", Serializer::toJson(root)},
        {"data.csv", Serializer::toCsv(rows)},
        {"data.xml", Serializer::toXml(rows, collection)},
        {"data.yaml", Serializer::toYaml(root)},
    };
    std::vector<std::string> created;
    for (auto& [name, content] : files) {
        fs::path path = fs::path(outDir) / name;
        writeFile(path, content);
        created.push_back(path.generic_string());
    }
    return created;
}

std::vector<std::string> exportLessons(const std::string& outDir) {
    // Таблица lessons связана с users (тренер, 1:N) и через bookings с клиентами (M:N).
    const std::string sql = R"SQL(
        SELECT l.lesson_id    AS "id",
               l.title        AS "title",
               l.starts_at    AS "startsAt",
               l.capacity     AS "capacity",
               t.user_id      AS "trainer.id",
               t.full_name    AS "trainer.fullName",
               t.phone        AS "trainer.phone",
               (SELECT COUNT(*) FROM bookings b
                 WHERE b.lesson_id = l.lesson_id AND b.status_id = 1) AS "bookedCount",
               (SELECT group_concat(c.full_name, ';')
                  FROM bookings b JOIN users c ON c.user_id = b.client_id
                 WHERE b.lesson_id = l.lesson_id AND b.status_id = 1) AS "clients[]"
        FROM lessons l
        JOIN users t ON t.user_id = l.trainer_id
        ORDER BY l.starts_at
    )SQL";
    return exportQuery(sql, "lessons", outDir);
}

}  // namespace Exporter
