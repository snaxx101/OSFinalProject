#include "chat_packet.h"
#include <sstream>
#include <iomanip>

std::string ChatPacket::to_string() const {
    std::ostringstream oss;
    std::tm* tm_info = std::localtime(&timestamp);
    oss << "[" << std::put_time(tm_info, "%Y-%m-%d %H:%M:%S") << "] "
        << sender << ": " << message;
    return oss.str();
}
