#ifndef GROUP_MANAGER_H
#define GROUP_MANAGER_H

#include <unordered_map>
#include <vector>
#include <deque>
#include <mutex>
#include <chrono>
#include "chat_packet.h"

class GroupManager {
private:
    struct CachedMessage {
        ChatPacket pkt;
        std::chrono::steady_clock::time_point timestamp;
    };

    std::unordered_map<int, std::deque<CachedMessage>> groups;
    std::unordered_map<int, size_t> maxMessagesPerGroup;
    std::mutex group_lock;
    std::chrono::seconds ttl = std::chrono::seconds(300); // default 5 min TTL

    void evictExpired(int groupID);

public:
    void setMaxMessages(int groupID, size_t maxMsgs);
    void addMessage(int groupID, const ChatPacket& pkt);
    std::vector<ChatPacket> getRecent(int groupID);
    std::vector<int> listGroups();

    void setTTL(int seconds); // optional setter for TTL
};

#endif // GROUP_MANAGER_H
