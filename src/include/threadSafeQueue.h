#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <iostream>

#include "parse.h"
#include "logger.h"


// Потокобезопасная очередь для задач логирования 
template<typename T>
class ThreadSafeQueue {
    public:
        void push(const T& item);

        // Блокировка для извлечения элемента 
        T waitAndPop();
    private:
        std::queue<T> queue;
        std::mutex mtx;
        std::condition_variable cv;
};

void logWorker(ThreadSafeQueue<logTask>& queue, Logger& logger);


#include "../threadSafeQueue.tpp"