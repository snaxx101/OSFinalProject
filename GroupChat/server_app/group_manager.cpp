#include "group_manager.h"

void GroupManager::addMessage(int groupID, const std::string& sender, const std::string& message) {
    std::lock_guard<std::mutex> lock(group_lock);
    groups[groupID].push_back({sender, message});
}

std::vector<ChatPacket> GroupManager::get_recent(int groupID) {
    std::lock_guard<std::mutex> lock(group_lock);
    return groups[groupID];
}
