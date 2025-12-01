#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <mutex>

class Logger {
private:
    std::ofstream ofs;
    std::mutex log_lock;

public:
    Logger(const std::string& filename);
    ~Logger();

    void log_message(int groupID, const std::string& sender, const std::string& message);
};

#endif // LOGGER_H
