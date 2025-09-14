#include <session.h>
#include <session_manager.h>
#include <tools/logger/logger_storage.h>
#include <spdlog/spdlog.h>

extern SessionManager session_manager;

Session::Session(asio::ip::tcp::socket socket)
    : socket_(std::move(socket)) {}

void Session::start() {
    spdlog::get("Session")->info("Session started");
    session_manager.add(shared_from_this());
    read();
}

void Session::read() {
  auto self(shared_from_this());
  socket_.async_read_some(asio::buffer(data_, buffer_size),
                          [this, self](const asio::error_code error_code, size_t length) {
                            spdlog::get("Session")->trace("Received data");
                            if (!error_code) {
                              std::string received_data(data_, length);
                              spdlog::get("Session")->trace("Received data: {}", received_data);
                              spdlog::get("Session")->trace("Received {} bytes", length);

                              if constexpr(buffer_size > 0) {
                                  session_manager.broadcast(data_);
                              }
                            } else {
                              if (error_code == asio::error::eof) {
                                spdlog::get("Session")->info("Client disconnected gracefully");
                              } else {
                                spdlog::get("Session")->warn("Read error: {}", error_code.message());
                              }
                            }
                          });
}

void Session::deliver(const std::string& message) {
    const bool write_in_progress = !outgoing_message_.empty();
    outgoing_message_ += message;

    if (!write_in_progress) {
        write(outgoing_message_);
    }
}

void Session::write(const std::string& message) {
    auto self(shared_from_this());
    asio::async_write(socket_, asio::buffer(message),
        [this, self, message](const asio::error_code ec, size_t /*length*/) {
            if (!ec) {
                outgoing_message_.erase(0, message.size());
                if (!outgoing_message_.empty()) {
                    write(outgoing_message_);
                }
            } else {
                spdlog::get("Session")->warn("Write error: {}", ec.message());
                session_manager.remove(shared_from_this());
            }
        });
}

void Session::close() {
    asio::error_code ec;
    socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
    socket_.close(ec);
    if (ec) {
        spdlog::get("Session")->warn("Error closing socket: {}", ec.message());
    }
}