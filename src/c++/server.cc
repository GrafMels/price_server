// server.cpp
#include <server.h>
#include <session.h>
#include <session_manager.h>
#include <tools/logger/logger_storage.h>
#include <spdlog/spdlog.h>

Server::Server(asio::io_context& io_context, short port)
    : acceptor_(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
{
  spdlog::get("Server")->info("Acceptor ready on port {}", port);
  accept();
}

void Server::accept() {
  acceptor_.async_accept(
      [this](asio::error_code ec, asio::ip::tcp::socket socket) {
          if (!ec) {
      const auto remote_ep = socket.remote_endpoint();
              spdlog::get("Server")->info("New connection from {}:{}",
                  remote_ep.address().to_string(), remote_ep.port());

              std::make_shared<Session>(std::move(socket))->start();
          } else {
              spdlog::get("Server")->error("Accept error: {}", ec.message());
          }
          accept();
      });
}