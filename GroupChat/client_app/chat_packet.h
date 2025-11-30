#ifndef CHAT_PACKET_H
#define CHAT_PACKET_H

#include <string>
#include <ctime>

class ChatPacket {
public:
    std::string sender;
    std::string message;
    std::time_t timestamp;

    ChatPacket() = default;

    ChatPacket(const std::string& sender, const std::string& message)
        : sender(sender), message(message), timestamp(std::time(nullptr)) {}

    std::string to_string() const;
};

#endif // CHAT_PACKET_H
