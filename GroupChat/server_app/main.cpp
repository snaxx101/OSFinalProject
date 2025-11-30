#include "chat_server.h"
#include "group_manager.h"
#include <iostream>
#include <thread>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server listening on port 8080...\n";

    while ((new_socket = accept(server_fd, (struct sockaddr*)&address,
                                (socklen_t*)&addrlen))) {
        std::cout << "New client connected\n";
        std::thread(handle_client, new_socket).detach();
    }

    if (new_socket < 0) {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }

    close(server_fd);
    return 0;
}
