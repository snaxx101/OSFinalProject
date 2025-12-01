#include "logger.h"
#include <ctime>
#include <iomanip>
#include <sstream>

Logger::Logger(const std::string& filename) {
    ofs.open(filename, std::ios::app);
    if (!ofs.is_open()) {
        throw std::runtime_error("Failed to open log file");
    }
}

Logger::~Logger() {
    if (ofs.is_open()) ofs.close();
}

void Logger::log_message(int groupID, const std::string& sender, const std::string& message) {
    std::lock_guard<std::mutex> lock(log_lock);
    auto now = std::time(nullptr);
    std::tm* tm_info = std::localtime(&now);

    std::ostringstream oss;
    oss << "[" << std::put_time(tm_info, "%Y-%m-%d %H:%M:%S") << "] "
        << "Group " << groupID << " - " << sender << ": " << message << "\n";

    ofs << oss.str();
    ofs.flush(); // ensure immediate write
}
