#include <iostream>
#include <string>
#include <thread>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <ctime>
#include "chat_packet.h"

void receiveMessages(int sock) {
    ChatPacketBinary pkt;
    while (true) {
        int r = recv(sock, &pkt, sizeof(pkt), 0);
        if (r <= 0) break;
        std::string sender(pkt.sender);
        std::string msg(pkt.payload, pkt.payloadLen);
        uint16_t gid = ntohs(pkt.groupID);
        std::cout << "[Group " << gid << "] " << sender << ": " << msg << "\n";
    }
}

int main() {
    std::string username;
    std::cout << "Enter your username: ";
    std::getline(std::cin, username);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(12345);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect"); return 1;
    }

    std::thread(receiveMessages, sock).detach();

    std::string msg;
    uint16_t lastGroup = 1;

    while (true) {
        std::getline(std::cin, msg);
        if (msg == "/quit") break;

        ChatPacketBinary pkt{};
        if (msg.rfind("/create", 0) == 0) {
            pkt = makePacket(PKT_CREATE_GROUP, std::stoi(msg.substr(8)), username, "");
        } else if (msg.rfind("/join", 0) == 0) {
            lastGroup = std::stoi(msg.substr(6));
            pkt = makePacket(PKT_JOIN_GROUP, lastGroup, username, "");
        } else if (msg == "/list") {
            pkt = makePacket(PKT_LIST_GROUPS, 0, username, "");
        } else {
            pkt = makePacket(PKT_MESSAGE, lastGroup, username, msg);
        }

        send(sock, &pkt, sizeof(pkt), 0);
    }

    close(sock);
    return 0;
}
