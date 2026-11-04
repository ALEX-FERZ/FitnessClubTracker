#pragma once
// Вспомогательные функции ввода-вывода консоли (Presentation Layer).

#include <optional>
#include <string>
#include <vector>

namespace Console {
// Включает UTF-8 в консоли Windows (вывод и ввод кириллицы).
void setup();

// Читает строку. При конце ввода (Ctrl+Z / EOF) бросает InputClosed.
std::string readLine(const std::string& prompt);
// Читает целое число; повторяет запрос, пока не введено корректное значение.
int readInt(const std::string& prompt);
// Пустой ввод -> nullopt.
std::optional<int> readOptionalInt(const std::string& prompt);
bool confirm(const std::string& question);

void title(const std::string& text);
void success(const std::string& text);
void error(const std::string& text);

// Печать таблицы с выравниванием по ширине (с учётом UTF-8).
void table(const std::vector<std::string>& header, const std::vector<std::vector<std::string>>& rows);

struct InputClosed {};
}  // namespace Console
