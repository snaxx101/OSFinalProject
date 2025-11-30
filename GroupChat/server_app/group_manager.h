#ifndef GROUP_MANAGER_H
#define GROUP_MANAGER_H

#include <unordered_map>
#include <vector>
#include <mutex>
#include "chat_packet.h"

class GroupManager {
private:
    std::unordered_map<int, std::vector<ChatPacket>> groups;
    std::mutex group_lock;

public:
    void addMessage(int groupID, const std::string& sender, const std::string& message);
    std::vector<ChatPacket> get_recent(int groupID);
};

#endif // GROUP_MANAGER_H
