#include "export/Exporter.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include "database/Database.h"

namespace {

struct Value {
    std::string text;
    bool isNumber = false;
    bool isNull = false;
    bool isList = false;
    std::vector<std::string> list;
};

// Поле записи: простое значение или вложенный объект (group) со своими полями
struct Field {
    std::string name;
    Value value;
    bool isGroup = false;
    std::vector<std::pair<std::string, Value>> children;
};

using Row = std::vector<Field>;

std::vector<std::string> split(const std::string& s) {
    std::vector<std::string> parts;
    std::string current;
    for (char c : s) {
        if (c == ';') { parts.push_back(current); current.clear(); }
        else current += c;
    }
    if (!current.empty()) parts.push_back(current);
    return parts;
}

// Выполняем запрос и превращаем каждую строку результата в Row
std::vector<Row> loadRows(const std::string& sql) {
    std::vector<Row> rows;
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cout << "Ошибка SQL: " << sqlite3_errmsg(db) << "\n";
        return rows;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Row row;
        for (int i = 0; i < sqlite3_column_count(stmt); i++) {
            std::string name = sqlite3_column_name(stmt, i);
            int type = sqlite3_column_type(stmt, i);
            Value v;
            v.text = columnText(stmt, i);
            v.isNull = type == SQLITE_NULL;
            v.isNumber = type == SQLITE_INTEGER || type == SQLITE_FLOAT;
            if (name.size() > 2 && name.substr(name.size() - 2) == "[]") {
                name = name.substr(0, name.size() - 2);
                v.isList = true;
                v.list = split(v.text);
            }

            size_t dot = name.find('.');
            if (dot == std::string::npos) {
                row.push_back({name, v});
            } else {
                std::string group = name.substr(0, dot);
                if (row.empty() || !row.back().isGroup || row.back().name != group)
                    row.push_back({group, {}, true, {}});
                row.back().children.push_back({name.substr(dot + 1), v});
            }
        }
        rows.push_back(row);
    }
    sqlite3_finalize(stmt);
    return rows;
}

std::string quote(const std::string& s) {
    std::string out = "\"";
    for (char c : s) {
        if (c == '"' || c == '\\') out += '\\';
        out += c;
    }
    return out + "\"";
}

// ---------- JSON ----------
std::string jsonValue(const Value& v) {
    if (v.isList) {
        std::string out = "[";
        for (size_t i = 0; i < v.list.size(); i++) out += (i ? ", " : "") + quote(v.list[i]);
        return out + "]";
    }
    if (v.isNull) return "null";
    if (v.isNumber) return v.text;
    return quote(v.text);
}

std::string toJson(const std::vector<Row>& rows, const std::string& table) {
    std::string out = "{\n  \"" + table + "\": [\n";
    for (size_t r = 0; r < rows.size(); r++) {
        out += "    {\n";
        for (size_t f = 0; f < rows[r].size(); f++) {
            const Field& field = rows[r][f];
            out += "      " + quote(field.name) + ": ";
            if (field.isGroup) {
                out += "{";
                for (size_t c = 0; c < field.children.size(); c++)
                    out += (c ? ", " : " ") + quote(field.children[c].first) + ": " + jsonValue(field.children[c].second);
                out += " }";
            } else {
                out += jsonValue(field.value);
            }
            out += (f + 1 < rows[r].size() ? ",\n" : "\n");
        }
        out += (r + 1 < rows.size() ? "    },\n" : "    }\n");
    }
    return out + "  ]\n}\n";
}

// ---------- CSV ----------
std::string csvCell(const Value& v) {
    std::string s;
    if (v.isList) {
        for (size_t i = 0; i < v.list.size(); i++) s += (i ? ";" : "") + v.list[i];
    } else {
        s = v.text;
    }
    if (s.find_first_of(",;\"") == std::string::npos) return s;
    std::string out = "\"";
    for (char c : s) out += (c == '"') ? std::string("\"\"") : std::string(1, c);
    return out + "\"";
}

std::string toCsv(const std::vector<Row>& rows) {
    std::string out = "\xEF\xBB\xBF";  // BOM, чтобы Excel правильно показал русские буквы
    if (rows.empty()) return out;
    std::string header;
    for (const Field& f : rows[0]) {
        if (f.isGroup)
            for (auto& c : f.children) header += (header.empty() ? "" : ",") + f.name + "." + c.first;
        else
            header += (header.empty() ? "" : ",") + f.name;
    }
    out += header + "\n";
    for (const Row& row : rows) {
        std::string line;
        bool first = true;
        for (const Field& f : row) {
            if (f.isGroup) {
                for (auto& c : f.children) { line += (first ? "" : ",") + csvCell(c.second); first = false; }
            } else {
                line += (first ? "" : ",") + csvCell(f.value);
                first = false;
            }
        }
        out += line + "\n";
    }
    return out;
}

// ---------- XML ----------
std::string xmlEscape(const std::string& s) {
    std::string out;
    for (char c : s) {
        if (c == '&') out += "&amp;";
        else if (c == '<') out += "&lt;";
        else if (c == '>') out += "&gt;";
        else out += c;
    }
    return out;
}

std::string xmlValue(const std::string& name, const Value& v, const std::string& indent) {
    if (v.isList) {
        // clients -> client
        std::string item = name.size() > 1 && name.back() == 's' ? name.substr(0, name.size() - 1) : "item";
        std::string out = indent + "<" + name + ">\n";
        for (auto& s : v.list) out += indent + "  <" + item + ">" + xmlEscape(s) + "</" + item + ">\n";
        return out + indent + "</" + name + ">\n";
    }
    if (v.isNull) return indent + "<" + name + "/>\n";
    return indent + "<" + name + ">" + xmlEscape(v.text) + "</" + name + ">\n";
}

std::string toXml(const std::vector<Row>& rows, const std::string& table) {
    std::string item = table.size() > 1 && table.back() == 's' ? table.substr(0, table.size() - 1) : "item";
    std::string out = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<" + table + ">\n";
    for (const Row& row : rows) {
        out += "  <" + item + ">\n";
        for (const Field& f : row) {
            if (f.isGroup) {
                out += "    <" + f.name + ">\n";
                for (auto& c : f.children) out += xmlValue(c.first, c.second, "      ");
                out += "    </" + f.name + ">\n";
            } else {
                out += xmlValue(f.name, f.value, "    ");
            }
        }
        out += "  </" + item + ">\n";
    }
    return out + "</" + table + ">\n";
}

// ---------- YAML ----------
std::string yamlValue(const std::string& name, const Value& v, const std::string& indent) {
    if (v.isList) {
        if (v.list.empty()) return indent + name + ": []\n";
        std::string out = indent + name + ":\n";
        for (auto& s : v.list) out += indent + "  - " + quote(s) + "\n";
        return out;
    }
    if (v.isNull) return indent + name + ": null\n";
    return indent + name + ": " + (v.isNumber ? v.text : quote(v.text)) + "\n";
}

std::string toYaml(const std::vector<Row>& rows, const std::string& table) {
    std::string out = table + ":\n";
    for (const Row& row : rows) {
        std::string block;
        for (const Field& f : row) {
            if (f.isGroup) {
                block += "    " + f.name + ":\n";
                for (auto& c : f.children) block += yamlValue(c.first, c.second, "      ");
            } else {
                block += yamlValue(f.name, f.value, "    ");
            }
        }
        out += "  - " + block.substr(4);  // первое поле записи идёт после "- "
    }
    return out;
}

void save(const std::string& path, const std::string& text) {
    std::ofstream file(path, std::ios::binary);
    file << text;
    std::cout << "Создан файл " << path << "\n";
}

}

namespace Exporter {

void exportQuery(const std::string& sql, const std::string& tableName, const std::string& outDir) {
    std::filesystem::create_directories(outDir);  // создаём папку out, если её нет
    std::vector<Row> rows = loadRows(sql);
    save(outDir + "/data.json", toJson(rows, tableName));
    save(outDir + "/data.csv", toCsv(rows));
    save(outDir + "/data.xml", toXml(rows, tableName));
    save(outDir + "/data.yaml", toYaml(rows, tableName));
}

void exportLessons(const std::string& outDir) {
    exportQuery(
        "SELECT l.LessonID AS id, l.title AS title, l.startTime AS startTime, l.maxPlaces AS maxPlaces, "
        "       u.UserID AS \"trainer.id\", u.fullName AS \"trainer.fullName\", u.phone AS \"trainer.phone\", "
        "       (SELECT group_concat(c.fullName, ';') FROM Booking b JOIN User c ON c.UserID = b.ClientID "
        "         WHERE b.LessonID = l.LessonID AND b.status = 1) AS \"clients[]\" "
        "FROM Lesson l JOIN User u ON u.UserID = l.TrainerID ORDER BY l.startTime",
        "lessons", outDir);
}

}
