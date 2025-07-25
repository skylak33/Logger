#include <iostream>
#include <stdexcept>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "socketSink.h"

void SocketSink::init() {
}
// Функция для очистки ресурсов
void SocketSink::cleanup() {
    if (sock >= 0) {
        close(sock);
        sock = -1;
    }
}

// Функция для подключения к серверу
void SocketSink::connectToServer(const std::string& address, int port) {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        throw std::runtime_error("Could not create socket");
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, address.c_str(), &serverAddr.sin_addr) <= 0) {
        close(sock);
        throw std::runtime_error("Invalid address or address not supported");
    }

    if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        close(sock);
        throw std::runtime_error("Connection failed");
    }
}

// Конструктор
SocketSink::SocketSink(const std::string& address, int port) {
    try {
        init();
        connectToServer(address, port);
    } catch (const std::exception& e) {
        std::cerr << "SocketSink failed: " << e.what() << std::endl;
        cleanup();
        throw;
    }
}

// Деструктор
SocketSink::~SocketSink() {
    if (sock >= 0) {
        close(sock);
    }
    cleanup();
}

// Функция записи сообщения в сокет
void SocketSink::write(const std::string& message) {
    if (sock >= 0) {
        std::string msg_with_newline = message + "\n";
        send(sock, msg_with_newline.c_str(), msg_with_newline.length(), 0);
    }
}