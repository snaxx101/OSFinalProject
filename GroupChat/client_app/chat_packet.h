#ifndef CHAT_PACKET_H
#define CHAT_PACKET_H

#include <cstdint>
#include <cstring>
#include <ctime>
#include <string>
#include <arpa/inet.h>
#include <sstream>
#include <iomanip>

constexpr size_t MAX_SENDER = 32;
constexpr size_t MAX_PAYLOAD = 256;

enum PacketType : uint8_t {
    PKT_CREATE_GROUP = 1,
    PKT_JOIN_GROUP = 2,
    PKT_MESSAGE = 3,
    PKT_LIST_GROUPS = 4,
    PKT_SERVER_MSG = 5
};

// Binary packet for sending over network
struct ChatPacketBinary {
    uint8_t type;
    uint16_t groupID;
    uint32_t timestamp;
    char sender[MAX_SENDER]{};
    char payload[MAX_PAYLOAD]{};
    size_t payloadLen;
};

// Helper to create a binary packet
inline ChatPacketBinary makePacket(uint8_t type, uint16_t groupID,
                                   const std::string &sender,
                                   const std::string &msg) {
    ChatPacketBinary p{};
    p.type = type;
    p.groupID = htons(groupID);
    p.timestamp = htonl(static_cast<uint32_t>(time(nullptr)));
    std::strncpy(p.sender, sender.c_str(), MAX_SENDER - 1);
    std::strncpy(p.payload, msg.c_str(), MAX_PAYLOAD - 1);
    p.payloadLen = msg.size() > MAX_PAYLOAD - 1 ? MAX_PAYLOAD - 1 : msg.size();
    return p;
}

// Human-readable chat packet for logging/display
class ChatPacket {
public:
    std::string sender;
    std::string message;
    std::time_t timestamp;

    ChatPacket() = default;

    ChatPacket(const std::string& sender, const std::string& message)
        : sender(sender), message(message), timestamp(std::time(nullptr)) {}

    std::string to_string() const; // defined in chat_packet.cpp
};

#endif // CHAT_PACKET_H
