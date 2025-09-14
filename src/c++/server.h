#pragma once

#include <asio.hpp>

class Server
{
 public:
  Server(asio::io_context& io_context, short port);

 private:
  void accept();

  asio::ip::tcp::acceptor acceptor_;
};
