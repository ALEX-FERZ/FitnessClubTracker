#include <cstdio>
#include <sstream>

#include "export/Exporter.h"

namespace {

std::string scalarToString(const DataNode& n) {
    switch (n.type) {
        case DataNode::Type::Null: return "";
        case DataNode::Type::Bool: return n.boolValue ? "true" : "false";
        case DataNode::Type::Int: return std::to_string(n.intValue);
        case DataNode::Type::Real: {
            char buf[64];
            std::snprintf(buf, sizeof(buf), "%.15g", n.realValue);
            return buf;
        }
        case DataNode::Type::Text: return n.textValue;
        default: return "";
    }
}

std::string jsonEscape(const std::string& s) {
    std::string out = "\"";
    for (unsigned char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (c < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else {
                    out += static_cast<char>(c);  // UTF-8 записываем как есть
                }
        }
    }
    return out + "\"";
}

std::string jsonScalar(const DataNode& n) {
    if (n.type == DataNode::Type::Null) return "null";
    if (n.type == DataNode::Type::Text) return jsonEscape(n.textValue);
    return scalarToString(n);
}

void writeJson(std::ostringstream& out, const DataNode& n, int indent) {
    std::string pad(indent, ' '), padIn(indent + 2, ' ');
    if (n.type == DataNode::Type::Object) {
        if (n.fields.empty()) { out << "{}"; return; }
        out << "{\n";
        for (std::size_t i = 0; i < n.fields.size(); ++i) {
            out << padIn << jsonEscape(n.fields[i].first) << ": ";
            writeJson(out, n.fields[i].second, indent + 2);
            out << (i + 1 < n.fields.size() ? ",\n" : "\n");
        }
        out << pad << "}";
    } else if (n.type == DataNode::Type::Array) {
        if (n.items.empty()) { out << "[]"; return; }
        out << "[\n";
        for (std::size_t i = 0; i < n.items.size(); ++i) {
            out << padIn;
            writeJson(out, n.items[i], indent + 2);
            out << (i + 1 < n.items.size() ? ",\n" : "\n");
        }
        out << pad << "]";
    } else {
        out << jsonScalar(n);
    }
}

// ---------- CSV ----------

void flatten(const DataNode& n, const std::string& prefix,
             std::vector<std::pair<std::string, std::string>>& out) {
    if (n.type == DataNode::Type::Object) {
        for (auto& [k, v] : n.fields) flatten(v, prefix.empty() ? k : prefix + "." + k, out);
    } else if (n.type == DataNode::Type::Array) {
        std::string joined;
        for (std::size_t i = 0; i < n.items.size(); ++i) {
            if (i) joined += ";";
            joined += n.items[i].isScalar() ? scalarToString(n.items[i]) : "[объект]";
        }
        out.emplace_back(prefix, joined);
    } else {
        out.emplace_back(prefix, scalarToString(n));
    }
}

std::string csvField(const std::string& s) {
    if (s.find_first_of(",;\"\n\r") == std::string::npos) return s;
    std::string out = "\"";
    for (char c : s) out += (c == '"') ? std::string("\"\"") : std::string(1, c);
    return out + "\"";
}

// ---------- XML ----------

std::string xmlEscape(const std::string& s) {
    std::string out;
    for (char c : s) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&apos;"; break;
            default: out += c;
        }
    }
    return out;
}

// clients -> client, lessons -> lesson; иначе item.
std::string singular(const std::string& tag) {
    if (tag.size() > 1 && tag.back() == 's') return tag.substr(0, tag.size() - 1);
    return "item";
}

void writeXml(std::ostringstream& out, const DataNode& n, const std::string& tag, int indent) {
    std::string pad(indent, ' ');
    if (n.type == DataNode::Type::Null) {
        out << pad << "<" << tag << "/>\n";
    } else if (n.isScalar()) {
        out << pad << "<" << tag << ">" << xmlEscape(scalarToString(n)) << "</" << tag << ">\n";
    } else if (n.isEmptyContainer()) {
        out << pad << "<" << tag << "/>\n";
    } else {
        out << pad << "<" << tag << ">\n";
        if (n.type == DataNode::Type::Object)
            for (auto& [k, v] : n.fields) writeXml(out, v, k, indent + 2);
        else
            for (auto& item : n.items) writeXml(out, item, singular(tag), indent + 2);
        out << pad << "</" << tag << ">\n";
    }
}

// ---------- YAML ----------

std::string yamlScalar(const DataNode& n) {
    if (n.type == DataNode::Type::Null) return "null";
    if (n.type == DataNode::Type::Text) return jsonEscape(n.textValue);  // "..." — валидный YAML
    if (n.type == DataNode::Type::Object) return "{}";
    if (n.type == DataNode::Type::Array) return "[]";
    return scalarToString(n);
}

void writeYaml(std::ostringstream& out, const DataNode& n, int indent) {
    std::string pad(indent, ' ');
    if (n.type == DataNode::Type::Object) {
        for (auto& [k, v] : n.fields) {
            out << pad << k << ":";
            if (v.isScalar() || v.isEmptyContainer()) {
                out << " " << yamlScalar(v) << "\n";
            } else {
                out << "\n";
                writeYaml(out, v, indent + 2);
            }
        }
    } else if (n.type == DataNode::Type::Array) {
        for (auto& item : n.items) {
            if (item.isScalar() || item.isEmptyContainer()) {
                out << pad << "- " << yamlScalar(item) << "\n";
            } else {
                // Элемент-объект: первая строка начинается с "- ", остальные выровнены.
                std::ostringstream inner;
                writeYaml(inner, item, indent + 2);
                std::string s = inner.str();
                out << pad << "- " << s.substr(indent + 2);
            }
        }
    } else {
        out << pad << yamlScalar(n) << "\n";
    }
}

}  // namespace

namespace Serializer {

std::string toJson(const DataNode& node) {
    std::ostringstream out;
    writeJson(out, node, 0);
    out << "\n";
    return out.str();
}

std::string toCsv(const DataNode& rows) {
    std::vector<std::string> header;
    std::vector<std::vector<std::pair<std::string, std::string>>> flat;
    for (auto& row : rows.items) {
        flat.emplace_back();
        flatten(row, "", flat.back());
        for (auto& [k, v] : flat.back()) {
            bool known = false;
            for (auto& h : header) known = known || h == k;
            if (!known) header.push_back(k);
        }
    }
    std::ostringstream out;
    out << "\xEF\xBB\xBF";  // BOM — чтобы Excel правильно открыл кириллицу
    for (std::size_t i = 0; i < header.size(); ++i) out << (i ? "," : "") << csvField(header[i]);
    out << "\r\n";
    for (auto& row : flat) {
        for (std::size_t i = 0; i < header.size(); ++i) {
            std::string value;
            for (auto& [k, v] : row)
                if (k == header[i]) value = v;
            out << (i ? "," : "") << csvField(value);
        }
        out << "\r\n";
    }
    return out.str();
}

std::string toXml(const DataNode& node, const std::string& rootTag) {
    std::ostringstream out;
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    writeXml(out, node, rootTag, 0);
    return out.str();
}

std::string toYaml(const DataNode& node) {
    std::ostringstream out;
    writeYaml(out, node, 0);
    return out.str();
}

}  // namespace Serializer
