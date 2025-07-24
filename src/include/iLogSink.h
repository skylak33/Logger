#pragma once

#include <string>

//Абстракция для логирования
class iLogSink {
    public:
        virtual ~iLogSink() = default;
        
        virtual void write(const std::string& message) = 0;
};