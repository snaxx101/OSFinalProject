#ifndef GROUP_MANAGER_H
#define GROUP_MANAGER_H

#include <unordered_map>
#include <deque>
#include <vector>
#include <mutex>
#include <chrono>
#include "chat_packet.h"

class GroupManager {
private:
    struct CachedMessage {
        ChatPacketBinary pkt;
        std::chrono::steady_clock::time_point ts;
    };

    std::unordered_map<int, std::deque<CachedMessage>> groups;
    std::unordered_map<int, size_t> maxMsgs;
    std::mutex lock;
    std::chrono::seconds ttl{300};

    void evictExpired(int groupID);

public:
    void setMax(int groupID, size_t maxMessages);
    void add(int groupID, const ChatPacketBinary &pkt);
    std::vector<ChatPacketBinary> recent(int groupID);
    std::vector<int> list();
    void setTTL(int seconds) { ttl = std::chrono::seconds(seconds); }
};

#endif
