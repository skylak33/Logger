#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <atomic>
#include <map>
#include <deque>
#include <limits>
#include <algorithm>
#include <sstream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define closesocket close

// --- Структура для хранения статистики ---
struct Statistics {
    std::atomic<long long> total_messages{0};
    std::map<std::string, std::atomic<long long>> level_counts;
    std::atomic<long long> min_length{std::numeric_limits<long long>::max()};
    std::atomic<long long> max_length{0};
    std::atomic<double> total_length_sum{0.0};
    std::atomic<bool> changed{false};

    // Для подсчета сообщений за последний час
    std::deque<std::chrono::time_point<std::chrono::system_clock>> hourly_timestamps;
    
    // Мьютекс для защиты 
    std::mutex mtx;

    Statistics() {
        level_counts["DEBUG"] = 0;
        level_counts["INFO"] = 0;
        level_counts["WARNING"] = 0;
        level_counts["ERROR"] = 0;
        level_counts["UNKNOWN"] = 0;
    }
};


// Функция для парсинга уровня и сообщения из строки лога
std::pair<std::string, std::string> parse_log_message(const std::string& line) {
    size_t start = line.find('[');
    size_t end = line.find(']');
    if (start == std::string::npos || end == std::string::npos || start > end) {
        return {"UNKNOWN", line};
    }
    std::string level = line.substr(start + 1, end - start - 1);
    std::string message = line.substr(end + 2); // +2 чтобы пропустить ']' и пробел
    return {level, message};
}

// Функция для вывода статистики
void print_statistics(Statistics& stats) {
    std::lock_guard<std::mutex> lock(stats.mtx);

    // Обновляем счетчик сообщений за последний час
    auto one_hour_ago = std::chrono::system_clock::now() - std::chrono::hours(1);
    while (!stats.hourly_timestamps.empty() && stats.hourly_timestamps.front() < one_hour_ago) {
        stats.hourly_timestamps.pop_front();
    }
    
    long long total = stats.total_messages.load();
    double avg_len = (total == 0) ? 0 : stats.total_length_sum.load() / total;

    std::cout << "\n--- СТАТИСТИКА ---" << std::endl;
    std::cout << "Сообщений всего: " << total << std::endl;
    std::cout << "Сообщений за последний час: " << stats.hourly_timestamps.size() << std::endl;
    std::cout << "По уровням:" << std::endl;
    std::cout << "  - DEBUG:   " << stats.level_counts["DEBUG"].load() << std::endl;
    std::cout << "  - INFO:    " << stats.level_counts["INFO"].load() << std::endl;
    std::cout << "  - WARNING: " << stats.level_counts["WARNING"].load() << std::endl;
    std::cout << "  - ERROR:   " << stats.level_counts["ERROR"].load() << std::endl;
    std::cout << "Длина сообщений:" << std::endl;
    std::cout << "  - Мин:     " << (total > 0 ? std::to_string(stats.min_length.load()) : "N/A") << std::endl;
    std::cout << "  - Макс:    " << (total > 0 ? std::to_string(stats.max_length.load()) : "N/A") << std::endl;
    std::cout << "  - Средняя: " << avg_len << std::endl;
    std::cout << "------------------\n" << std::endl;

    stats.changed = false;
}



// Поток-обработчик для каждого клиента
void client_handler(int client_socket, Statistics& stats, int n_messages_trigger) {
    char buffer[4096];
    while (true) {
        int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            std::cout << "[Клиент отключился]" << std::endl;
            break;
        }
        buffer[bytes_received] = '\0';
        
        // Обработка нескольких сообщений в одном пакете
        std::stringstream ss(buffer);
        std::string line;
        while (std::getline(ss, line)) {
            if (line.empty()) continue;

            std::cout << "Принято: " << line << std::endl;

            //Обновляем статистику
            auto [level, message] = parse_log_message(line);
            long long msg_len = message.length();

            // Блокируем обновления
            {
                std::lock_guard<std::mutex> lock(stats.mtx);
                stats.level_counts[level]++;
                stats.hourly_timestamps.push_back(std::chrono::system_clock::now());
            }

            stats.total_length_sum = stats.total_length_sum + msg_len;
            if (msg_len < stats.min_length) stats.min_length = msg_len;
            if (msg_len > stats.max_length) stats.max_length = msg_len;
            stats.changed = true;
            long long total_count = ++stats.total_messages;

            //  Проверка
            if (n_messages_trigger > 0 && total_count % n_messages_trigger == 0) {
                print_statistics(stats);
            }
        }
    }
    closesocket(client_socket);
}

// Поток-отчетчик, который выводит статистику по таймауту T
void stats_reporter(Statistics& stats, int t_seconds_trigger) {
    if (t_seconds_trigger <= 0) return;
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(t_seconds_trigger));
        if (stats.changed.load()) {
            std::cout << "[Вывод статистики по таймауту...]" << std::endl;
            print_statistics(stats);
        }
    }
}

// --- Основная функция ---
int main(int argc, char* argv[]) {
    // 1. Парсинг аргументов
    if (argc != 5) {
        std::cerr << "Использование: " << argv[0] << " <хост> <порт> <N> <T>\n";
        std::cerr << "  <хост>: IP-адрес для прослушивания (например, 127.0.0.1)\n";
        std::cerr << "  <порт>: Порт для прослушивания (например, 9999)\n";
        std::cerr << "  <N>:    Выводить статистику после каждого N-го сообщения (0 для отключения)\n";
        std::cerr << "  <T>:    Выводить статистику по таймауту T секунд (0 для отключения)\n";
        return 1;
    }
    std::string host = argv[1];
    int port = std::stoi(argv[2]);
    int N = std::stoi(argv[3]);
    int T = std::stoi(argv[4]);

    Statistics stats;
    

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        std::cerr << "Не удалось создать сокет\n";
        return 1;
    }
    
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Bind failed\n";
        return 1;
    }

    listen(server_socket, 5);
    std::cout << "Сервер статистики запущен на " << host << ":" << port << std::endl;
    std::cout << "Триггер по сообщениям (N): " << N << std::endl;
    std::cout << "Триггер по таймауту (T): " << T << " сек." << std::endl;

    // поток-отчетчика
    std::thread reporter_thread(stats_reporter, std::ref(stats), T);
    reporter_thread.detach(); // Запускаем в фоновом режиме

    while (true) {
        int client_socket = accept(server_socket, nullptr, nullptr);
        if (client_socket < 0) {
            std::cerr << "Accept failed\n";
            continue;
        }
        std::cout << "[Подключился новый клиент]" << std::endl;
        
        // поток-обработчик
        std::thread handler_thread(client_handler, client_socket, std::ref(stats), N);
        handler_thread.detach();
    }

    closesocket(server_socket);
    return 0;
}