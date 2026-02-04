#ifndef SHAWNCOIN_UTIL_REALTIME_HPP
#define SHAWNCOIN_UTIL_REALTIME_HPP

#include <string>

namespace shawncoin {

// Append a single-line JSON event to the realtime feed file (~/.shawncoin/realtime_feed.log).
void appendRealtimeEvent(const std::string& jsonLine);

} // namespace shawncoin

#endif // SHAWNCOIN_UTIL_REALTIME_HPP
