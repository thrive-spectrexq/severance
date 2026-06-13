#pragma once

#include <memory>
#include <spdlog/spdlog.h>

namespace severance::core::logging {

class Logger {
public:
  static void Init();

  static std::shared_ptr<spdlog::logger> &GetCoreLogger() {
    return s_CoreLogger;
  }
  static std::shared_ptr<spdlog::logger> &GetClientLogger() {
    return s_ClientLogger;
  }

private:
  static std::shared_ptr<spdlog::logger> s_CoreLogger;
  static std::shared_ptr<spdlog::logger> s_ClientLogger;
};

} // namespace severance::core::logging

// Core log macros
#define SEV_CORE_TRACE(...)                                                    \
  ::severance::core::logging::Logger::GetCoreLogger()->trace(__VA_ARGS__)
#define SEV_CORE_INFO(...)                                                     \
  ::severance::core::logging::Logger::GetCoreLogger()->info(__VA_ARGS__)
#define SEV_CORE_WARN(...)                                                     \
  ::severance::core::logging::Logger::GetCoreLogger()->warn(__VA_ARGS__)
#define SEV_CORE_ERROR(...)                                                    \
  ::severance::core::logging::Logger::GetCoreLogger()->error(__VA_ARGS__)
#define SEV_CORE_CRITICAL(...)                                                 \
  ::severance::core::logging::Logger::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define SEV_TRACE(...)                                                         \
  ::severance::core::logging::Logger::GetClientLogger()->trace(__VA_ARGS__)
#define SEV_INFO(...)                                                          \
  ::severance::core::logging::Logger::GetClientLogger()->info(__VA_ARGS__)
#define SEV_WARN(...)                                                          \
  ::severance::core::logging::Logger::GetClientLogger()->warn(__VA_ARGS__)
#define SEV_ERROR(...)                                                         \
  ::severance::core::logging::Logger::GetClientLogger()->error(__VA_ARGS__)
#define SEV_CRITICAL(...)                                                      \
  ::severance::core::logging::Logger::GetClientLogger()->critical(__VA_ARGS__)
