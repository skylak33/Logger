#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <sstream>

#include "logger.h"

struct logTask {
    std::optional<logLevel> level;
    std::string message;
};
std::string toLowerCase(const std::string& str) {
    std::string result = str;
    std::ranges::transform(result, result.begin(),
        [](unsigned char c){ return std::tolower(c); });
    return result;
}

// Потокобезопасная очередь для задач логирования 
template<typename T>
class ThreadSafeQueue {
    public:
        void push(const T& item) {
            std::lock_guard<std::mutex> lock(mtx);
            queue.push(item);
            cv.notify_one(); // Уведомляем поток, о новом элементе
        }

        // Блокировка для извлечения элемента 
        T waitAndPop() {
            std::unique_lock<std::mutex> lock(mtx);
            // ждем пока очередь не опустеет
            cv.wait(lock, [this] { return !queue.empty(); });
            T item = std::move(queue.front());
            queue.pop();
            return item;
        }

    private:
        std::queue<T> queue;
        std::mutex mtx;
        std::condition_variable cv;
};

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
    string lvl = toLowerCase(level);
    if (lvl == "debug")       return logLevel::DEBUG;
    if (lvl == "info")        return logLevel::INFO;
    if (lvl == "warning")     return logLevel::WARNING;
    if (lvl == "error")       return logLevel::ERROR;
    
    std::cerr << "Неверный уровень лога '" << level << "'. Используется INFO." << std::endl;
    return logLevel::INFO;
}
// Функция, выполняемая в отдельном потоке
void logWorker(ThreadSafeQueue<logTask>& queue, Logger& logger) {
    std::cout << "[logger]: Запущен." << std::endl;
    while (true) {
        logTask task = queue.waitAndPop();
        if (task.message == "exit") {
            break;
        }
        if (task.level) {   // Если в задаче указан уровень, используем его.
            logger.log(*task.level, task.message);
        }   else {
            logger.logInfo(task.message);
        }
        std::cout << "[logger]: Завершение работы." << std::endl;
    }
}

void print_usage() {
    std::cout << "Использование: ./logger_app <имя_файла> <уровень_по_умолчанию>\n";
    std::cout << "  <имя_файла>:              Путь к файлу журнала.\n";
    std::cout << "  <уровень_по_умолчанию>:  debug, info, warning, error\n";
}

int main(int argc, char* argv[]) {

    if (argc != 3) {
        print_usage();
        return 1;
    }

    std::string filename = argv[1];
    logLevel default_level = parseArgs(argv[2]);

    // Создаем логгер и очередь
    try {
        auto logger = Logger::createFileLogger(filename, default_level);
        ThreadSafeQueue<logTask> queue;

        // Запускаем поток логирования
        std::thread worker(logWorker, std::ref(queue), std::ref(logger));

        std::cout << "Приложение готово. Введите сообщение для записи в журнал.\n";
        std::cout << "Формат: [уровень] <сообщение> (уровень необязателен).\n";
        std::cout << "Для выхода введите 'exit'.\n";
        std::string input;
        while (std::getline(std::cin, input)) {
            if (input == "exit") {
                queue.push({std::nullopt, "exit"}); // Отправляем задачу для завершения потока
                break;
            }

            if (input.empty()) {
                continue; // Игнорируем пустые строки
            }

            logTask task = parseInput(input);
            queue.push(task); // Добавляем задачу в очередь
        }
        
        worker.join();
        std::cout << "Приложение завершено." << std::endl;

    }   catch (const std::exception& e) {
        std::cerr << "Ошибка при создании логгера: " << e.what() << std::endl;
        return 1;
    }


    return 0;
}
