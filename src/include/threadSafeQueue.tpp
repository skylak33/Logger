#include "threadSafeQueue.h"

template<typename T>
void ThreadSafeQueue<T>::push(const T& item) {
    std::lock_guard<std::mutex> lock(mtx);
    queue.push(item);
    cv.notify_one(); // Уведомляем поток, о новом элементе
}

template<typename T>
T ThreadSafeQueue<T>::waitAndPop() {
    std::unique_lock<std::mutex> lock(mtx);
    // ждем пока очередь не опустеет
    cv.wait(lock, [this] { return !queue.empty(); });
    T item = std::move(queue.front());
    queue.pop();
    return item;
}

// Функция, выполняемая в отдельном потоке
void logWorker(ThreadSafeQueue<logTask>& queue, Logger& logger) {
    std::cout << "[logger thread]: Запущен." << std::endl;
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
        std::cout << "[logger thread]: Завершение работы." << std::endl;
    }
}