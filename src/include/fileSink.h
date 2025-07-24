#pragma once
#include <string>
#include <fstream>

#include "iLogSink.h"

class FileSink : public iLogSink {
    public:
        explicit FileSink(const std::string& filename);
        ~FileSink() override;

        void write(const std::string& message) override;
    private:
        std::ofstream fileStream;
};