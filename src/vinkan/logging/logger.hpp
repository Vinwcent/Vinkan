#ifndef VINKAN_LOGGER_HPP
#define VINKAN_LOGGER_HPP

#include <memory>

#ifdef DEVELOPMENT_BUILD
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

#else

namespace vinkan {
struct NullLogger {};
} // namespace vinkan

#ifndef SPDLOG_LOGGER_TRACE
#define SPDLOG_LOGGER_TRACE(...) ((void)0)
#endif

#ifndef SPDLOG_LOGGER_DEBUG
#define SPDLOG_LOGGER_DEBUG(...) ((void)0)
#endif

#ifndef SPDLOG_LOGGER_INFO
#define SPDLOG_LOGGER_INFO(...) ((void)0)
#endif

#ifndef SPDLOG_LOGGER_WARN
#define SPDLOG_LOGGER_WARN(...) ((void)0)
#endif

#ifndef SPDLOG_LOGGER_ERROR
#define SPDLOG_LOGGER_ERROR(...) ((void)0)
#endif

#ifndef SPDLOG_LOGGER_CRITICAL
#define SPDLOG_LOGGER_CRITICAL(...) ((void)0)
#endif

inline std::shared_ptr<vinkan::NullLogger> get_vinkan_logger() { return {}; }

#endif

#endif
