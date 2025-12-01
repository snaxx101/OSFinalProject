#include <iostream>
#include <string>
#include <thread>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

struct ChatPacketBinary {
    uint8_t type;
    uint16_t groupID;
    uint32_t timestamp;
    char payload[256];
};

// Receive thread
void receive_messages(int sock) {
    ChatPacketBinary bpkt;
    while (true) {
        int bytes_received = recv(sock, &bpkt, sizeof(bpkt), 0);
        if (bytes_received <= 0) break;

        if (bpkt.type == 0) { // text
            uint16_t gid = ntohs(bpkt.groupID);
            uint32_t ts = ntohl(bpkt.timestamp);
            std::cout << "[Group " << gid << "] " << bpkt.payload << "\n";
        }
    }
}

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket failed"); return 1; }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect failed"); return 1;
    }

    std::cout << "Connected to server!\n";

    std::thread(receive_messages, sock).detach();

    std::string msg;
    while (true) {
        std::getline(std::cin, msg);
        if (msg == "/quit") break;

        // Send as binary packet
        ChatPacketBinary bpkt{};
        bpkt.type = 0;
        bpkt.groupID = htons(1); // default group
        bpkt.timestamp = htonl(static_cast<uint32_t>(time(nullptr)));
        std::memset(bpkt.payload, 0, sizeof(bpkt.payload));
        std::strncpy(bpkt.payload, msg.c_str(), sizeof(bpkt.payload)-1);

        send(sock, &bpkt, sizeof(bpkt), 0);
    }

    close(sock);
    return 0;
}
