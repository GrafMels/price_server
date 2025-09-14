#pragma once

#include <memory>
#include <set>
#include <mutex>
#include <session.h>

class SessionManager {
public:
  void add(const std::shared_ptr<Session>& session);
  void remove(const std::shared_ptr<Session>& session);
  void broadcast(const std::string& message);
  void shutdown();

private:
  std::set<std::shared_ptr<Session>> sessions_;
  std::mutex mutex_;
};