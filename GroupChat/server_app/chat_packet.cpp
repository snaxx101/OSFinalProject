#include "chat_packet.h"
#include <sstream>
#include <iomanip>
#include <ctime>

std::string ChatPacket::to_string() const {
    std::ostringstream oss;
    std::time_t t = static_cast<std::time_t>(timestamp);
    std::tm tm_info{};
    localtime_r(&t, &tm_info);
    oss << "[" << std::put_time(&tm_info, "%Y-%m-%d %H:%M:%S") << "] "
        << sender << ": " << message;
    return oss.str();
}
