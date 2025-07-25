#include <chrono>
#include <iomanip>
#include <sstream>

#include "logger.h"
#include "fileSink.h"
#include "socketSink.h"

Logger::Logger(std::unique_ptr<iLogSink> sink, logLevel level)
    : sink(std::move(sink)), currentLevel(level) {
}

Logger Logger::createFileLogger(const std::string& filename, logLevel defaultLevel) {
    auto sink = std::make_unique<FileSink>(filename);
    return Logger(std::move(sink), defaultLevel);
}

Logger Logger::createSocketLogger(const std::string& address, int port, logLevel defaultLevel) {
    auto sink = std::make_unique<SocketSink>(address, port);
    return Logger(std::move(sink), defaultLevel);
}

void Logger::setLevel(logLevel level) {
    std::lock_guard<std::mutex> lock(mtx);
    currentLevel = level;
}

void Logger::log(logLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(mtx);

    if (level < currentLevel || !sink) {
        return; // Игнорируем сообщения ниже текущего уровня или если нет приемника
    }
    
    // Получаем текущее время
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::localtime(&now_c);

    // Форматируем сообщение
    std::ostringstream oss;
    oss << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S") << " [" 
        << logLevelToString(level) << "] " << message;

    // Записываем сообщение в приемник
    sink->write(oss.str());


}

// Реализация вспомогательных методов
void Logger::logDebug(const std::string& message) {
    log(logLevel::DEBUG, message);
}
void Logger::logInfo(const std::string& message) {
    log(logLevel::INFO, message);
}
void Logger::logWarning(const std::string& message) {
    log(logLevel::WARNING, message);
}
void Logger::logError(const std::string& message) {
    log(logLevel::ERROR, message);
}