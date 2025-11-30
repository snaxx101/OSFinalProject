#include "chat_client.h"

int main() {
    ChatClient client("127.0.0.1", 8080);
    client.start();
    return 0;
}
