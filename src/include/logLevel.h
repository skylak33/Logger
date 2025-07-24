#pragma once

#include <string>
// Уровни важности сообщений
enum class logLevel {
    DEBUG = 0,
    INFO,       // Информационные сообщения
    WARNING,    // Предупреждения
    ERROR       // Ошибки
};
// функция для преобразования уровня в строку
inline std::string logLevelToString(logLevel level) {
    switch (level) {
        case logLevel::DEBUG:       return "DEBUG";
        case logLevel::INFO:        return "INFO";
        case logLevel::WARNING:     return "WARNING";
        case logLevel::ERROR:       return "ERROR";
        default:                    return "UNKNOWN";
    }
}