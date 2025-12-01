#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <netinet/in.h>
#include <unistd.h>
#include <algorithm>
#include <fstream>
#include <cstring>
#include "group_manager.h"
#include "chat_packet.h"

// Binary protocol structure
struct ChatPacketBinary {
    uint8_t type;        // 0 = text
    uint16_t groupID;
    uint32_t timestamp;
    char payload[256];
};

GroupManager groupManager;

std::vector<int> connected_clients;
std::mutex clients_lock;
std::unordered_map<int, int> client_groups;
std::mutex client_group_lock;
std::mutex log_lock;

// Log a message
void log_message(const ChatPacket& pkt, int groupID) {
    std::lock_guard<std::mutex> lock(log_lock);
    std::ofstream log_file("logs/chat_log.txt", std::ios::app);
    if (log_file.is_open()) {
        log_file << "[" << groupID << "] " << pkt.to_string() << "\n";
    }
}

// Broadcast binary message to all clients in the group
void broadcast_message_binary(int groupID, const ChatPacketBinary& bpkt) {
    std::lock_guard<std::mutex> lock(clients_lock);
    for (int client_socket : connected_clients) {
        std::lock_guard<std::mutex> cg_lock(client_group_lock);
        auto it = client_groups.find(client_socket);
        if (it != client_groups.end() && it->second == groupID) {
            send(client_socket, &bpkt, sizeof(bpkt), 0);
        }
    }
}

// Send recent text messages to client (human-readable)
void send_recent_messages(int client_socket, int groupID) {
    auto recent = groupManager.getRecent(groupID);
    if (recent.empty()) return;

    std::string recentStr = "=== Recent messages ===\n";
    for (auto it = recent.rbegin(); it != recent.rend(); ++it) {
        recentStr += it->to_string() + "\n";
    }
    send(client_socket, recentStr.c_str(), recentStr.size(), 0);
}

void handle_client(int client_socket) {
    {
        std::lock_guard<std::mutex> lock(clients_lock);
        connected_clients.push_back(client_socket);
    }

    std::string welcome = "Connected to Chat Server!\n";
    send(client_socket, welcome.c_str(), welcome.size(), 0);

    char buffer[1024];
    int bytes_received;
    int current_group = -1;

    while ((bytes_received = recv(client_socket, buffer, sizeof(buffer)-1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        std::string msg(buffer);

        // Text commands
        if (msg.rfind("/create", 0) == 0) {
            int groupID = std::stoi(msg.substr(8));
            groupManager.setMaxMessages(groupID, 10);
            std::string response = "Group " + std::to_string(groupID) + " created.\n";
            send(client_socket, response.c_str(), response.size(), 0);
        } else if (msg.rfind("/join", 0) == 0) {
            int groupID = std::stoi(msg.substr(6));
            {
                std::lock_guard<std::mutex> cg_lock(client_group_lock);
                client_groups[client_socket] = groupID;
            }
            current_group = groupID;
            send_recent_messages(client_socket, groupID);
            std::string response = "Joined group " + std::to_string(groupID) + ".\n";
            send(client_socket, response.c_str(), response.size(), 0);
        } else if (msg.rfind("/list", 0) == 0) {
            auto groups = groupManager.listGroups();
            std::string response = "Active groups: ";
            for (int g : groups) response += std::to_string(g) + " ";
            response += "\n";
            send(client_socket, response.c_str(), response.size(), 0);
        } 
        // Normal text message -> convert to binary packet
        else if (current_group != -1) {
            ChatPacket pkt("User", msg);
            groupManager.addMessage(current_group, pkt);
            log_message(pkt, current_group);

            // Create binary packet
            ChatPacketBinary bpkt{};
            bpkt.type = 0;
            bpkt.groupID = htons(current_group);
            bpkt.timestamp = htonl(static_cast<uint32_t>(pkt.timestamp));
            std::memset(bpkt.payload, 0, sizeof(bpkt.payload));
            std::strncpy(bpkt.payload, pkt.message.c_str(), sizeof(bpkt.payload)-1);

            broadcast_message_binary(current_group, bpkt);
        }
    }

    // Remove client on disconnect
    {
        std::lock_guard<std::mutex> lock(clients_lock);
        connected_clients.erase(
            std::remove(connected_clients.begin(), connected_clients.end(), client_socket),
            connected_clients.end()
        );
    }

    {
        std::lock_guard<std::mutex> cg_lock(client_group_lock);
        client_groups.erase(client_socket);
    }

    close(client_socket);
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) { perror("socket failed"); exit(EXIT_FAILURE); }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(12345);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed"); exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) { perror("listen failed"); exit(EXIT_FAILURE); }

    std::cout << "Server running on port 12345...\n";

    while (true) {
        int addrlen = sizeof(address);
        int client_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (client_socket < 0) { perror("accept failed"); continue; }
        std::thread(handle_client, client_socket).detach();
    }

    return 0;
}
