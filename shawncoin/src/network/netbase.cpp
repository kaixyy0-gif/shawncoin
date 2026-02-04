#include "network/netbase.hpp"
#include <algorithm>
#include <cctype>

namespace shawncoin {

bool CNetAddr::isIPv4() const {
    return std::find(host.begin(), host.end(), ':') == host.end() &&
           std::find(host.begin(), host.end(), '.') != host.end();
}

bool CNetAddr::isIPv6() const {
    return host.find(':') != std::string::npos;
}

} // namespace shawncoin
