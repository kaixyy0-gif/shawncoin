#include "util/realtime.hpp"
#include <fstream>
#include <cstdlib>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>

namespace shawncoin {

static std::string defaultDataDir() {
    const char* home = std::getenv("HOME");
    if (!home) home = "/tmp";
    std::string dir = std::string(home) + "/.shawncoin";
    // try to create if missing
    struct stat st;
    if (stat(dir.c_str(), &st) != 0) {
        // attempt create
        mkdir(dir.c_str(), 0700);
    }
    return dir;
}

void appendRealtimeEvent(const std::string& jsonLine) {
    std::string dir = defaultDataDir();
    std::string path = dir + "/realtime_feed.log";
    std::ofstream f(path, std::ios::app);
    if (!f) return;
    f << jsonLine << "\n";
}

} // namespace shawncoin
