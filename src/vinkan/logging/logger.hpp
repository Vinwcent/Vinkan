#ifndef VINKAN_LOGGER_HPP
#define VINKAN_LOGGER_HPP

#define SPDLOG_ACTIVE_LEVEL 1
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

inline std::shared_ptr<spdlog::logger> get_vinkan_logger() {
  static auto logger = []() {
    auto console_logger = spdlog::stdout_color_mt("vinkan");
    console_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [vinkan] [%l] %v");
    console_logger->set_level(spdlog::level::trace);
    return console_logger;
  }();
  return logger;
}

static auto vinkan_logger = get_vinkan_logger();

#endif

