#include "ui/Input.h"
#include <cstdlib>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

void setupConsole() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}

std::string readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
#ifdef _WIN32
    // В консоли Windows читаем строку в UTF-16 и переводим в UTF-8,
    // иначе русские буквы при вводе превращаются в мусор.
    HANDLE in = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    if (GetConsoleMode(in, &mode)) {
        wchar_t buffer[512];
        DWORD count = 0;
        if (!ReadConsoleW(in, buffer, 511, &count, nullptr) || count == 0) std::exit(0);
        while (count > 0 && (buffer[count - 1] == L'\n' || buffer[count - 1] == L'\r')) count--;
        int size = WideCharToMultiByte(CP_UTF8, 0, buffer, count, nullptr, 0, nullptr, nullptr);
        line.resize(size);
        WideCharToMultiByte(CP_UTF8, 0, buffer, count, &line[0], size, nullptr, nullptr);
        return line;
    }
#endif
    if (!std::getline(std::cin, line)) std::exit(0);  // ввод закончился
    if (!line.empty() && line.back() == '\r') line.pop_back();
    return line;
}

int readInt(const std::string& prompt) {
    while (true) {
        std::string s = readLine(prompt);
        try {
            return std::stoi(s);
        } catch (...) {
            std::cout << "Введите число\n";
        }
    }
}
