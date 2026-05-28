#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <filesystem>
#include <fstream>
#include <mutex>

namespace FileMove {

class Logger {
public:
    static void LogSession(const std::filesystem::path& logPath, const std::string& message);
    static void LogTransfer(const std::filesystem::path& logPath, 
                            const std::string& fileName, 
                            const std::filesystem::path& sourceDir, 
                            const std::filesystem::path& destDir, 
                            const std::string& timestamp, 
                            const std::string& result);

    static void LogDebug(const std::string& message);
    static void LogError(const std::string& error);
    static void SetDebugEnabled(bool enabled);

private:
    static std::mutex s_logMutex;
    static bool s_debugEnabled;
};

} // namespace FileMove

#endif // LOGGER_HPP
