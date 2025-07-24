#pragma once

#include <string>
#include <memory>
#include <mutex>

#include "logLevel.h"
#include "iLogSink.h"

class Logger {
    public:
        // метод для создания логгера, пишущего в файл
        static Logger createFileLogger(const std::string& filename, logLevel defaultLevel = logLevel::INFO);
        
        // метод для создания логгера, пишущего в сокет
        static Logger createSocketLogger(const std::string& address, int port, logLevel defaultLevel = logLevel::INFO);

        void setLevel(logLevel level);

        void log(logLevel level, const std::string& message);

        // вспомогательные методы
        void logDebug(const std::string& message);
        void logInfo(const std::string& message);
        void logWarning(const std::string& message);
        void logError(const std::string& message);

        // Перемещающий конструктор и оператор присваивания
        Logger(Logger&&) = default;
        Logger& operator=(Logger&&) = default;

    private:
        // Конструктор
        Logger(std::unique_ptr<iLogSink> sink, logLevel level);

        // Запрещаем копирование
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;

        std::unique_ptr<iLogSink> sink;
        logLevel currentLevel;
        std::mutex mtx;
};