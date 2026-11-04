#pragma once
// Универсальное дерево данных для сериализации (ЛР №5).
// Узел — скаляр (null/целое/дробное/строка/логическое), объект (упорядоченные пары
// ключ–значение) или массив. Все сериализаторы работают с этим деревом и ничего
// не знают о конкретных таблицах.

#include <string>
#include <utility>
#include <vector>

struct DataNode {
    enum class Type { Null, Bool, Int, Real, Text, Object, Array };

    Type type = Type::Null;
    bool boolValue = false;
    long long intValue = 0;
    double realValue = 0;
    std::string textValue;
    std::vector<std::pair<std::string, DataNode>> fields;  // для Object
    std::vector<DataNode> items;                           // для Array

    static DataNode null() { return {}; }
    static DataNode boolean(bool v) { DataNode n; n.type = Type::Bool; n.boolValue = v; return n; }
    static DataNode integer(long long v) { DataNode n; n.type = Type::Int; n.intValue = v; return n; }
    static DataNode real(double v) { DataNode n; n.type = Type::Real; n.realValue = v; return n; }
    static DataNode text(std::string v) { DataNode n; n.type = Type::Text; n.textValue = std::move(v); return n; }
    static DataNode object() { DataNode n; n.type = Type::Object; return n; }
    static DataNode array() { DataNode n; n.type = Type::Array; return n; }

    bool isScalar() const { return type != Type::Object && type != Type::Array; }
    bool isEmptyContainer() const {
        return (type == Type::Object && fields.empty()) || (type == Type::Array && items.empty());
    }

    // Возвращает поле объекта, создавая его при необходимости.
    DataNode& operator[](const std::string& key) {
        for (auto& f : fields)
            if (f.first == key) return f.second;
        fields.emplace_back(key, DataNode{});
        return fields.back().second;
    }
};
