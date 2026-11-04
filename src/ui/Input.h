#pragma once
#include <string>

// Ввод с клавиатуры (работает и с русскими буквами в консоли Windows)
void setupConsole();
std::string readLine(const std::string& prompt);
int readInt(const std::string& prompt);
