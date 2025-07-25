#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <memory>
#include <optional>


#include "logger.h"
#include "threadSafeQueue.h"
#include "parse.h"


int main(int argc, char* argv[]) {
    if (argc != 3) {
        print_usage();
        return 1;
    }

    std::string filename = argv[1];
    logLevel default_level = parseArgs(argv[2]);

    // Создаем логгер и очередь
    try {
        auto logger = Logger::createFileLogger(filename, default_level); // Часть 2
        // auto socket_logger = Logger::createSocketLogger("127.0.0.1", 9999, logLevel::DEBUG); // Часть 3
        ThreadSafeQueue<logTask> queue;

        // Запускаем поток логирования
        std::thread worker(logWorker, std::ref(queue), std::ref(logger); // Часть 2
        // std::thread worker(logWorker, std::ref(queue), std::ref(socket_logger));// Часть 3

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
        //std::cerr << "Ошибка при создании логгера: " << e.what() << std::endl;
        std::cerr << "Не удалось создать сокет-логгер: " << e.what() << std::endl;
        return 1;
    }


    return 0;
}
