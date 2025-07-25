#pragma once
#include <string>
#include <optional>

#include "logLevel.h"

struct logTask {
    std::optional<logLevel> level;
    std::string message;
};

std::string toLowerCase(const std::string& str);

// Функция для парсинга строки ввода от пользователя
logTask parseInput(const std::string& input);

// Функция для парса из аргуементов командной строки
logLevel parseArgs(const std::string& level);

void print_usage();