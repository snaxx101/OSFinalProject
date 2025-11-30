#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <netinet/in.h>
#include <unistd.h>
#include "group_manager.h"
#include "chat_packet.h"
#include <algorithm>

GroupManager groupManager;

std::vector<int> connected_clients;
std::mutex clients_lock;

void broadcast_message(const std::string& msg) {
    std::lock_guard<std::mutex> lock(clients_lock);
    for (int client_socket : connected_clients) {
        send(client_socket, msg.c_str(), msg.size(), 0);
    }
}

void handle_client(int client_socket) {
    {
        std::lock_guard<std::mutex> lock(clients_lock);
        connected_clients.push_back(client_socket);
    }

    // Send welcome message
    std::string welcome = "Connected to Chat Server!\n";
    send(client_socket, welcome.c_str(), welcome.size(), 0);

    char buffer[1024];
    int bytes_received;
    while ((bytes_received = recv(client_socket, buffer, sizeof(buffer)-1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        std::string msg(buffer);

        // For demo, assume groupID=1 and sender="User"
        groupManager.addMessage(1, "User", msg);

        std::string full_msg = "User: " + msg + "\n";
        broadcast_message(full_msg); // send to all clients
    }

    // Remove client on disconnect
    {
        std::lock_guard<std::mutex> lock(clients_lock);
        connected_clients.erase(
            std::remove(connected_clients.begin(), connected_clients.end(), client_socket),
            connected_clients.end()
        );
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
