#include "group_manager.h"
#include <algorithm>

void GroupManager::setMax(int groupID, size_t maxMessages) {
    std::lock_guard<std::mutex> g(lock);
    maxMsgs[groupID] = maxMessages;
    if (groups.find(groupID) == groups.end())
        groups[groupID] = {};
}

void GroupManager::evictExpired(int groupID) {
    auto &dq = groups[groupID];
    auto now = std::chrono::steady_clock::now();
    while (!dq.empty() && std::chrono::duration_cast<std::chrono::seconds>(now - dq.front().ts) > ttl) {
        dq.pop_front();
    }
}

void GroupManager::add(int groupID, const ChatPacketBinary &pkt) {
    std::lock_guard<std::mutex> g(lock);

    if (groups.find(groupID) == groups.end()) {
        groups[groupID] = {};
        maxMsgs[groupID] = 10;
    }

    evictExpired(groupID);

    groups[groupID].push_back({pkt, std::chrono::steady_clock::now()});

    while (groups[groupID].size() > maxMsgs[groupID])
        groups[groupID].pop_front();
}

std::vector<ChatPacketBinary> GroupManager::recent(int groupID) {
    std::lock_guard<std::mutex> g(lock);
    evictExpired(groupID);
    std::vector<ChatPacketBinary> res;
    for (auto &c : groups[groupID]) res.push_back(c.pkt);
    return res;
}

std::vector<int> GroupManager::list() {
    std::lock_guard<std::mutex> g(lock);
    std::vector<int> res;
    for (auto &p : groups) res.push_back(p.first);
    return res;
}
