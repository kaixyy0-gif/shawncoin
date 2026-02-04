#include "util/config.hpp"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <algorithm>

namespace shawncoin {

bool Config::load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    std::string section;
    std::string line;
    while (std::getline(f, line)) {
        size_t c = line.find('#');
        if (c != std::string::npos) line.resize(c);
        while (!line.empty() && (line.back() == '\r' || line.back() == ' ')) line.pop_back();
        if (line.empty()) continue;
        if (line[0] == '[') {
            size_t end = line.find(']');
            section = end != std::string::npos ? line.substr(1, end - 1) : "";
            continue;
        }
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);
        while (!key.empty() && key.back() == ' ') key.pop_back();
        while (!val.empty() && val.front() == ' ') val.erase(0, 1);
        if (!section.empty()) key = section + "." + key;
        values_[key] = val;
        if (key.find("addnode") != std::string::npos)
            addNodes_.push_back(val);
    }
    return true;
}

bool Config::loadFromArgs(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--testnet") { setTestnet(true); continue; }
        if (arg == "--mine") { values_["mine"] = "1"; continue; }
        if (arg == "--datadir" && i + 1 < argc) { values_["datadir"] = argv[++i]; continue; }
        if (arg.compare(0, 2, "--") == 0) {
            size_t eq = arg.find('=', 2);
            std::string key = eq != std::string::npos ? arg.substr(2, eq - 2) : arg.substr(2);
            std::string val = eq != std::string::npos ? arg.substr(eq + 1) : "";
            values_[key] = val;
        }
    }
    return true;
}

std::string Config::get(const std::string& key, const std::string& defaultValue) const {
    auto it = values_.find(key);
    return it != values_.end() ? it->second : defaultValue;
}

int Config::getInt(const std::string& key, int defaultValue) const {
    std::string v = get(key, "");
    if (v.empty()) return defaultValue;
    return std::stoi(v);
}

uint16_t Config::getPort(const std::string& key, uint16_t defaultPort) const {
    int p = getInt(key, (int)defaultPort);
    return (p > 0 && p < 65536) ? (uint16_t)p : defaultPort;
}

std::string Config::getDataDir() const {
    std::string d = get("datadir", "");
    if (!d.empty()) return d;
    const char* home = std::getenv("HOME");
    if (home) return std::string(home) + "/.shawncoin";
#ifdef _WIN32
    const char* app = std::getenv("APPDATA");
    if (app) return std::string(app) + "\\ShawnCoin";
#endif
    return ".shawncoin";
}

} // namespace shawncoin
