#include "ui/Console.h"

#include <algorithm>
#include <iostream>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace {

// Количество символов (а не байт) в UTF-8 строке.
std::size_t displayWidth(const std::string& s) {
    std::size_t n = 0;
    for (unsigned char c : s)
        if ((c & 0xC0) != 0x80) ++n;
    return n;
}

std::string pad(const std::string& s, std::size_t width) {
    return s + std::string(width > displayWidth(s) ? width - displayWidth(s) : 0, ' ');
}

#ifdef _WIN32
bool stdinIsConsole() {
    DWORD mode;
    return GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &mode) != 0;
}

// В консоли Windows читаем UTF-16 и переводим в UTF-8: так кириллица вводится корректно.
bool readConsoleLine(std::string& out) {
    std::wstring wide;
    wchar_t buf[256];
    DWORD read = 0;
    while (true) {
        if (!ReadConsoleW(GetStdHandle(STD_INPUT_HANDLE), buf, 256, &read, nullptr) || read == 0)
            return false;
        wide.append(buf, read);
        if (!wide.empty() && wide.back() == L'\n') break;
    }
    if (wide.size() == 1 && wide[0] == 0x1A) return false;  // Ctrl+Z
    while (!wide.empty() && (wide.back() == L'\n' || wide.back() == L'\r')) wide.pop_back();
    int len = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), static_cast<int>(wide.size()), nullptr,
                                  0, nullptr, nullptr);
    out.assign(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), static_cast<int>(wide.size()), out.data(), len,
                        nullptr, nullptr);
    return true;
}
#endif

}  // namespace

namespace Console {

void setup() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}

std::string readLine(const std::string& prompt) {
    std::cout << prompt << std::flush;
    std::string line;
#ifdef _WIN32
    if (stdinIsConsole()) {
        if (!readConsoleLine(line)) throw InputClosed{};
        return line;
    }
#endif
    if (!std::getline(std::cin, line)) throw InputClosed{};
    if (!line.empty() && line.back() == '\r') line.pop_back();
    return line;
}

std::optional<int> readOptionalInt(const std::string& prompt) {
    while (true) {
        std::string s = readLine(prompt);
        if (s.find_first_not_of(' ') == std::string::npos) return std::nullopt;
        try {
            std::size_t used = 0;
            int value = std::stoi(s, &used);
            if (s.find_first_not_of(' ', used) == std::string::npos) return value;
        } catch (...) {
        }
        error("Введите целое число");
    }
}

int readInt(const std::string& prompt) {
    while (true) {
        auto v = readOptionalInt(prompt);
        if (v) return *v;
        error("Значение обязательно");
    }
}

bool confirm(const std::string& question) {
    std::string a = readLine(question + " (д/н): ");
    return a == "д" || a == "Д" || a == "y" || a == "Y" || a == "да" || a == "Да";
}

void title(const std::string& text) { std::cout << "\n===== " << text << " =====\n"; }
void success(const std::string& text) { std::cout << "[OK] " << text << "\n"; }
void error(const std::string& text) { std::cout << "[Ошибка] " << text << "\n"; }

void table(const std::vector<std::string>& header,
           const std::vector<std::vector<std::string>>& rows) {
    if (rows.empty()) {
        std::cout << "(нет данных)\n";
        return;
    }
    std::vector<std::size_t> widths(header.size());
    for (std::size_t i = 0; i < header.size(); ++i) widths[i] = displayWidth(header[i]);
    for (auto& row : rows)
        for (std::size_t i = 0; i < row.size() && i < widths.size(); ++i)
            widths[i] = std::max(widths[i], displayWidth(row[i]));

    auto line = [&] {
        for (auto w : widths) std::cout << "+" << std::string(w + 2, '-');
        std::cout << "+\n";
    };
    auto print = [&](const std::vector<std::string>& r) {
        for (std::size_t i = 0; i < widths.size(); ++i)
            std::cout << "| " << pad(i < r.size() ? r[i] : "", widths[i]) << " ";
        std::cout << "|\n";
    };
    line();
    print(header);
    line();
    for (auto& r : rows) print(r);
    line();
}

}  // namespace Console
