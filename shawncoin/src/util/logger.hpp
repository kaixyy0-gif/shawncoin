#ifndef SHAWNCOIN_UTIL_LOGGER_HPP
#define SHAWNCOIN_UTIL_LOGGER_HPP

#include <string>
#include <cstdio>
#include <mutex>

namespace shawncoin {

enum class LogLevel { Debug, Info, Warn, Error, Critical };

class Logger {
public:
    static Logger& instance();
    void setLevel(LogLevel level) { level_ = level; }
    void setFile(const std::string& path);
    void log(LogLevel level, const char* component, const char* fmt, ...);

private:
    Logger() = default;
    std::mutex mutex_;
    LogLevel level_ = LogLevel::Info;
    FILE* file_ = nullptr;
    std::string path_;
};

#define SHAWNCOIN_LOG(level, component, ...) \
    shawncoin::Logger::instance().log(shawncoin::LogLevel::level, component, __VA_ARGS__)

} // namespace shawncoin

#endif // SHAWNCOIN_UTIL_LOGGER_HPP
