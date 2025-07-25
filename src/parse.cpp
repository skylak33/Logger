#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>


#include "parse.h"

std::string toLowerCase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c){ return std::tolower(c); });
    return result;
}

// Функция для парсинга строки ввода от пользователя
logTask parseInput(const std::string& input) {
    std::stringstream ss(input);
    std::string type;
    ss >> type;

    std::string rest;
    std::getline(ss, rest);

    if (type == "debug")          return {logLevel::DEBUG, rest};
    if (type == "info")           return {logLevel::INFO, rest};
    if (type == "warning")        return {logLevel::WARNING, rest};
    if (type == "error")          return {logLevel::ERROR, rest};

    std::cerr << "Неверный уровень лога '" << type << "'. Используется INFO." << std::endl;
    return {logLevel::INFO, rest}; 
}
// Функция для парса из аргуементов командной строки
logLevel parseArgs(const std::string& level) { 
    std::string lvl = toLowerCase(level);
    if (lvl == "debug")       return logLevel::DEBUG;
    if (lvl == "info")        return logLevel::INFO;
    if (lvl == "warning")     return logLevel::WARNING;
    if (lvl == "error")       return logLevel::ERROR;
    
    std::cerr << "Неверный уровень лога '" << level << "'. Используется INFO." << std::endl;
    return logLevel::INFO;
}

void print_usage() {
    std::cout << "Использование: ./logger_app <имя_файла> <уровень_по_умолчанию>\n";
    std::cout << "  <имя_файла>:              Путь к файлу журнала.\n";
    std::cout << "  <уровень_по_умолчанию>:  debug, info, warning, error\n";
}