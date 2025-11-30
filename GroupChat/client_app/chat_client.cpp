#include "chat_client.h"
#include <iostream>
#include <unistd.h>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

ChatClient::ChatClient(const std::string& ip, int p) : serverIP(ip), port(p) {}

void ChatClient::start() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cout << "Socket creation error" << std::endl;
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, serverIP.c_str(), &serv_addr.sin_addr) <= 0) {
        std::cout << "Invalid address/ Address not supported" << std::endl;
        return;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cout << "Connection failed" << std::endl;
        return;
    }

    // 1. Enter groupID
    std::string groupID;
    std::cout << "Enter groupID: ";
    std::getline(std::cin, groupID);
    send(sock, groupID.c_str(), groupID.size(), 0);

    // 2. Read recent messages from server
    int valread = read(sock, buffer, 1024);
    if (valread > 0) std::cout << buffer;

    // 3. Send chat message
    std::string message;
    std::cout << "Enter message: ";
    std::getline(std::cin, message);
    send(sock, message.c_str(), message.size(), 0);

    valread = read(sock, buffer, 1024);
    if (valread > 0) std::cout << buffer << std::endl;

    close(sock);
}
