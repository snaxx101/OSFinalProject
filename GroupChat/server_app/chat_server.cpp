#include <iostream>
#include <thread>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <algorithm>
#include <unistd.h>
#include <netinet/in.h>
#include <fstream>
#include "chat_packet.h"
#include "group_manager.h"

GroupManager groupManager;
std::vector<int> clients;
std::mutex clients_lock;

std::unordered_map<int, int> client_group;
std::mutex cg_lock;

void logMessage(const ChatPacketBinary &pkt, int groupID) {
    std::ofstream logf("logs/chat_log.txt", std::ios::app);
    if (!logf.is_open()) return;
    logf << "[Group " << groupID << "] " << pkt.sender << ": "
         << std::string(pkt.payload, pkt.payloadLen) << "\n";
}

void broadcast(int groupID, const ChatPacketBinary &pkt) {
    std::vector<int> local;
    { std::lock_guard<std::mutex> g(clients_lock); local = clients; }

    for (int sock : local) {
        bool inGroup = false;
        {
            std::lock_guard<std::mutex> g(cg_lock);
            if (client_group.count(sock) && client_group[sock] == groupID)
                inGroup = true;
        }
        if (inGroup) send(sock, &pkt, sizeof(pkt), 0);
    }
}

void sendRecent(int sock, int groupID) {
    auto msgs = groupManager.recent(groupID);
    for (auto &m : msgs) send(sock, &m, sizeof(m), 0);
}

void handleClient(int sock) {
    {
        std::lock_guard<std::mutex> g(clients_lock);
        clients.push_back(sock);
    }

    ChatPacketBinary pkt{};
    while (true) {
        int r = recv(sock, &pkt, sizeof(pkt), 0);
        if (r <= 0) break;

        uint16_t gid = ntohs(pkt.groupID);

        switch (pkt.type) {
            case PKT_CREATE_GROUP:
                groupManager.setMax(gid, 10);
                break;

            case PKT_JOIN_GROUP:
                {
                    std::lock_guard<std::mutex> g(cg_lock);
                    client_group[sock] = gid;
                }
                sendRecent(sock, gid);
                break;

            case PKT_MESSAGE:
                groupManager.add(gid, pkt);
                logMessage(pkt, gid);
                broadcast(gid, pkt);
                break;

            case PKT_LIST_GROUPS:
                {
                    auto list = groupManager.list();
                    std::string msg = "Groups: ";
                    for (int g : list) msg += std::to_string(g) + " ";

                    ChatPacketBinary out = makePacket(PKT_SERVER_MSG, 0, "server", msg);
                    send(sock, &out, sizeof(out), 0);
                }
                break;
        }
    }

    {
        std::lock_guard<std::mutex> g(clients_lock);
        clients.erase(std::remove(clients.begin(), clients.end(), sock), clients.end());
    }
    {
        std::lock_guard<std::mutex> g(cg_lock);
        client_group.erase(sock);
    }
    close(sock);
}

int main() {
    int server = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(12345);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server, (sockaddr*)&addr, sizeof(addr));
    listen(server, 10);

    std::cout << "Binary Chat Server running...\n";

    while (true) {
        int client = accept(server, nullptr, nullptr);
        std::thread(handleClient, client).detach();
    }

    return 0;
}
