#include <session_manager.h>
#include <tools/logger/logger_storage.h>
#include <spdlog/spdlog.h>

void SessionManager::add(const std::shared_ptr<Session>& session) {
  std::lock_guard<std::mutex> lock(mutex_);
  sessions_.insert(session);
  spdlog::get("SessionManager")->info("Session added. Total sessions: {}", sessions_.size());
}

void SessionManager::remove(const std::shared_ptr<Session>& session) {
  std::lock_guard<std::mutex> lock(mutex_);
  sessions_.erase(session);
  spdlog::get("SessionManager")->info("Session removed. Total sessions: {}", sessions_.size());
}

void SessionManager::broadcast(const std::string& message) {
  for (auto& session : sessions_) {
    session->deliver(message);
  }
}

void SessionManager::shutdown() {
  std::lock_guard<std::mutex> lock(mutex_);
  for (auto& session : sessions_) {
    session->close();
  }
  sessions_.clear();
  spdlog::get("SessionManager")->info("All sessions closed");
}