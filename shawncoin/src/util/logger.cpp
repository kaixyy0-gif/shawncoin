#include "util/logger.hpp"
#include <cstdarg>
#include <ctime>
#include <cstring>

namespace shawncoin {

Logger& Logger::instance() {
    static Logger log;
    return log;
}

void Logger::setFile(const std::string& path) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (file_ && file_ != stdout) fclose(file_);
    file_ = nullptr;
    path_ = path;
    if (!path.empty())
        file_ = fopen(path.c_str(), "a");
    if (!file_) file_ = stdout;
}

void Logger::log(LogLevel level, const char* component, const char* fmt, ...) {
    if (level < level_) return;
    const char* levelStr = "???";
    switch (level) {
        case LogLevel::Debug:    levelStr = "DEBUG"; break;
        case LogLevel::Info:     levelStr = "INFO"; break;
        case LogLevel::Warn:     levelStr = "WARN"; break;
        case LogLevel::Error:    levelStr = "ERROR"; break;
        case LogLevel::Critical: levelStr = "CRITICAL"; break;
    }
    time_t t = time(nullptr);
    struct tm tm_buf;
#ifdef _WIN32
    localtime_s(&tm_buf, &t);
#else
    localtime_r(&t, &tm_buf);
#endif
    char timeBuf[64];
    strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", &tm_buf);
    std::lock_guard<std::mutex> lock(mutex_);
    FILE* out = file_ ? file_ : stdout;
    fprintf(out, "%s [%s] [%s] ", timeBuf, levelStr, component);
    va_list ap;
    va_start(ap, fmt);
    vfprintf(out, fmt, ap);
    va_end(ap);
    fprintf(out, "\n");
    fflush(out);
}

} // namespace shawncoin
