#pragma once

#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <filesystem>

constexpr auto default_level = spdlog::level::level_enum::info;
constexpr auto default_name = "price_server";
static const auto file_postfix = [] {
  const auto& now = std::chrono::system_clock::now();
  const auto& time = std::chrono::system_clock::to_time_t(now);
  std::tm tm {};
  localtime_r(&time, &tm);
  std::stringstream ss;
  ss << "." << std::put_time(&tm, "%Y-%m-%d_%H:%M:%S.")
     << std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1'000 << ".txt";
  return ss.str();
}();

namespace tools::logger_storage
{
  namespace config
  {
    constexpr std::string_view log_pattern = "[%d-%m-%Y %H:%M:%S.%e] [%^%l%$] [%n] %v";
    constexpr size_t max_file_size_bytes = 1'024 * 1'024 * 10;
    constexpr size_t max_file_count = 5;
  }  // namespace config

  enum class Target
  {
    Stdout = 1 << 0,
    File = 1 << 1,
    All = Stdout | File
  };

  enum class LoggerCreationType
  {
    Default,
    Build,
    Rebuild
  };

  inline auto create_logger(const std::shared_ptr<spdlog::logger>& logger, const std::string& log_pattern) -> void
  {
    logger->set_level(spdlog::level::trace);
    logger->set_pattern(log_pattern);
    logger->flush_on(spdlog::level::trace);
    logger->flush();
  }

  inline auto build_log(
    const LoggerCreationType& logger_creation_type,
    const std::string& name,
    const Target& log_target,
    const std::string& input_file_name,
    const spdlog::level::level_enum& input_log_level
  ) -> void
  {
    if(name.empty())
      return;
    const auto& file_name = input_file_name.empty() ? default_name + file_postfix : input_file_name + file_postfix;
    const auto& log_level = input_log_level == spdlog::level::level_enum::n_levels ? default_level : input_log_level;

    const auto log_dir = std::filesystem::path(std::string("/var/log/droneport_server/") + default_name + "/" + "log");
    create_directories(log_dir);
    const auto log_file = log_dir / file_name;

    auto bitmask = static_cast<std::underlying_type_t<Target>>(log_target);
    std::vector<spdlog::sink_ptr> sinks;
    auto mask = 1;
    while(bitmask) {
      switch(bitmask & mask) {
        case static_cast<std::underlying_type_t<Target>>(Target::File): {
          auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            log_file,
            config::max_file_size_bytes,
            config::max_file_count
          );
          file_sink->set_level(spdlog::level::trace);
          file_sink->flush();
          sinks.push_back(file_sink);
          break;
        }
        case static_cast<std::underlying_type_t<Target>>(Target::Stdout): {
          auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
          console_sink->set_level(log_level);
          sinks.push_back(console_sink);
          break;
        }
        default: break;
      }
      bitmask &= ~mask;
      mask <<= 1;
    }

    switch(logger_creation_type) {
      case LoggerCreationType::Rebuild: spdlog::drop(name); [[fallthrough]];
      case LoggerCreationType::Build:
        initialize_logger(std::make_shared<spdlog::logger>(name.data(), begin(sinks), end(sinks)));
        create_logger(spdlog::get(name), std::string(config::log_pattern));
        break;
      case LoggerCreationType::Default:
      default:
        set_default_logger(std::make_shared<spdlog::logger>(name.data(), begin(sinks), end(sinks)));
        create_logger(spdlog::default_logger(), std::string(config::log_pattern));
        break;
    }
  }

  inline auto build_logs(const spdlog::level::level_enum& level) -> void
  {
    std::vector<std::string> names = {"Server", "Session", "SessionManager"};
    build_log(LoggerCreationType::Default, PROJECT_NAME, Target::All, default_name, level);
    for(const auto& name : names) {
      build_log(LoggerCreationType::Build, name, Target::All, default_name, level);
    }
    build_log(LoggerCreationType::Build, "Telemetry", Target::File, "telemetry", level);
  }
}  // namespace tools::logger_storage
