#ifndef CACHE_H
#define CACHE_H

#include <unordered_map>
#include <list>
#include <mutex>
#include <string>
#include <vector>
#include <ctime>

struct CachedMessage {
    uint32_t timestamp;
    std::string sender;
    std::string content;
};

class MessageCache {
private:
    size_t capacity;
    std::list<CachedMessage> messageList; // front = oldest
    std::mutex mtx;
    uint32_t ttlSeconds; // optional TTL

public:
    MessageCache(size_t cap, uint32_t ttl = 0) : capacity(cap), ttlSeconds(ttl) {}

    void addMessage(const CachedMessage& msg) {
        std::lock_guard<std::mutex> lock(mtx);

        // Remove expired messages if TTL is set
        if (ttlSeconds > 0) {
            auto now = static_cast<uint32_t>(time(nullptr));
            while (!messageList.empty() && now - messageList.front().timestamp > ttlSeconds) {
                messageList.pop_front();
            }
        }

        // Add new message
        messageList.push_back(msg);

        // Remove oldest if exceeding capacity
        if (messageList.size() > capacity) {
            messageList.pop_front();
        }
    }

    std::vector<CachedMessage> getRecentMessages() {
        std::lock_guard<std::mutex> lock(mtx);
        return std::vector<CachedMessage>(messageList.begin(), messageList.end());
    }
};

#endif // CACHE_H
