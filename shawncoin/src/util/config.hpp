#ifndef SHAWNCOIN_UTIL_CONFIG_HPP
#define SHAWNCOIN_UTIL_CONFIG_HPP

#include <string>
#include <map>
#include <vector>
#include <cstdint>

namespace shawncoin {

class Config {
public:
    bool load(const std::string& path);
    bool loadFromArgs(int argc, char* argv[]);
    std::string get(const std::string& key, const std::string& defaultValue = "") const;
    int getInt(const std::string& key, int defaultValue = 0) const;
    uint16_t getPort(const std::string& key, uint16_t defaultPort) const;
    std::string getDataDir() const;
    bool isTestnet() const { return testnet_; }
    void setTestnet(bool v) { testnet_ = v; }

private:
    std::map<std::string, std::string> values_;
    std::vector<std::string> addNodes_;
    bool testnet_ = false;
};

} // namespace shawncoin

#endif // SHAWNCOIN_UTIL_CONFIG_HPP
