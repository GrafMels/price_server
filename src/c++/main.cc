#include <tools/logger/logger_storage.h>
#include <server.h>
#include <session_manager.h>
#include <asio.hpp>
#include <vector>
#include <thread>
#include <csignal>
#include <atomic>

// Глобальный менеджер сессий
SessionManager session_manager;
std::atomic<bool> stop_server(false);

void init() {
    tools::logger_storage::build_logs(spdlog::level::level_enum::trace);
    spdlog::default_logger()->info("{}: {}", PROJECT_NAME, PROJECT_VERSION);
    spdlog::default_logger()->info("Developed by {} - {}", PROJECT_COMPANY, PROJECT_AUTHOR);
}

void handle_signals(asio::io_context& io_context) {
    asio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait([&](const asio::error_code& error_code, int signum) {
        if (!error_code) {
            spdlog::get("Server")->info("Received signal {}", signum);
            stop_server.store(true);

            // Очищаем все сессии перед остановкой
            session_manager.shutdown();

            io_context.stop();
        }
    });
}

int main(int argc, char* argv[]) {
    init();

    try {
        constexpr short port = 8080;
        asio::io_context io_context;

        // Настройка обработки сигналов
        handle_signals(io_context);

        // Создание сервера
        Server server(io_context, port);
        spdlog::get("Server")->info("Server started on port {}", port);

        // Запуск I/O сервиса в нескольких потоках
        std::vector<std::thread> threads;
        size_t thread_pool_size = std::thread::hardware_concurrency() * 2;

        for(size_t i = 0; i < thread_pool_size; ++i) {
            threads.emplace_back([&io_context](){
                try {
                    io_context.run();
                } catch (const std::exception& e) {
                    spdlog::get("Server")->critical("I/O context error: {}", e.what());
                }
            });
        }

        spdlog::get("Server")->info("Running with {} worker threads", thread_pool_size);

        // Ожидание завершения всех потоков
        for(auto& t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }

        spdlog::get("Server")->info("Server stopped gracefully");
    } catch (std::exception& e) {
        spdlog::get("Server")->critical("Server exception: {}", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}