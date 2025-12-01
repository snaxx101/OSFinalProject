#include "group_manager.h"
#include <algorithm>

void GroupManager::setMaxMessages(int groupID, size_t maxMsgs) {
    std::lock_guard<std::mutex> lock(group_lock);
    maxMessagesPerGroup[groupID] = maxMsgs;
}

void GroupManager::setTTL(int seconds) {
    ttl = std::chrono::seconds(seconds);
}

void GroupManager::evictExpired(int groupID) {
    auto& dequeRef = groups[groupID];
    auto now = std::chrono::steady_clock::now();

    while (!dequeRef.empty() && 
           std::chrono::duration_cast<std::chrono::seconds>(now - dequeRef.front().timestamp) > ttl) {
        dequeRef.pop_front();
    }
}

void GroupManager::addMessage(int groupID, const ChatPacket& pkt) {
    std::lock_guard<std::mutex> lock(group_lock);

    // Create group if it doesn't exist
    if (groups.find(groupID) == groups.end()) {
        groups[groupID] = std::deque<CachedMessage>();
        maxMessagesPerGroup[groupID] = 10; // default max messages
    }

    evictExpired(groupID);

    // Add new message
    groups[groupID].push_back({pkt, std::chrono::steady_clock::now()});

    // Enforce max messages per group (LRU)
    while (groups[groupID].size() > maxMessagesPerGroup[groupID]) {
        groups[groupID].pop_front();
    }
}

std::vector<ChatPacket> GroupManager::getRecent(int groupID) {
    std::lock_guard<std::mutex> lock(group_lock);
    evictExpired(groupID);

    std::vector<ChatPacket> recent;
    for (auto& cm : groups[groupID]) {
        recent.push_back(cm.pkt);
    }
    return recent;
}

std::vector<int> GroupManager::listGroups() {
    std::lock_guard<std::mutex> lock(group_lock);
    std::vector<int> groupIDs;
    for (auto& pair : groups) {
        groupIDs.push_back(pair.first);
    }
    return groupIDs;
}
