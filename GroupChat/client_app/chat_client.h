#ifndef CHAT_CLIENT_H
#define CHAT_CLIENT_H

#include <string>

class ChatClient {
private:
    std::string serverIP;
    int port;

public:
    ChatClient(const std::string& ip, int p);
    void start();
};

#endif // CHAT_CLIENT_H
