#pragma once

#include <string>

#include "iLogSink.h"

// Класс для записи логов в сокет
class SocketSink : public iLogSink {
    public:
        SocketSink(const std::string& address, int port);
        ~SocketSink() override;

        void write(const std::string& message) override;

    private:
        void init();
        void cleanup();
        void connectToServer(const std::string& address, int port);

        int sock = -1; // Сокет для соединения (-1 означает, что сокет не тот)
};

