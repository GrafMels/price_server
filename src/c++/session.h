#pragma once

#include <asio.hpp>
#include <memory>
#include <string>

class Session : public std::enable_shared_from_this<Session> {
public:
  explicit Session(asio::ip::tcp::socket socket);
  void start();
  void deliver(const std::string& message);
  void close();

private:
  void read();
  void write(const std::string& message);

  asio::ip::tcp::socket socket_;
  static constexpr size_t buffer_size = 1'024;
  char data_[buffer_size];
  std::string outgoing_message_;
  asio::streambuf incoming_buffer_;
};