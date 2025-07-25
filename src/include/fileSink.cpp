#include "fileSink.h"
#include <stdexcept>
// Конструктор
FileSink::FileSink(const std::string& filename) {
    fileStream.open(filename, std::ios::out | std::ios::app);
    if (!fileStream.is_open()) {
        throw std::runtime_error("Could not open log file: " + filename);
    }
}
// Деструктор
FileSink::~FileSink() = default;

// Функция записи сообщения в файл
void FileSink::write(const std::string& message) {
    if (fileStream.is_open()) {
        fileStream << message << std::endl;
    }
}