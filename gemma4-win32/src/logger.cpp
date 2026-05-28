#include "logger.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <mutex>
#include <vector>

namespace FileMove {

std::mutex Logger::s_logMutex;
bool Logger::s_debugEnabled = false;

void Logger::LogDebug(const std::string& message) {
    if (s_debugEnabled) {
        std::lock_guard<std::mutex> lock(s_logMutex);
        std::cout << "----> " << message << std::endl;
    }
}

void Logger::LogError(const std::string& error) {
    if (s_debugEnabled) {
        std::lock_guard<std::mutex> lock(s_logMutex);
        std::cerr << "[ERROR] " << error << std::endl;
    }
}

void Logger::SetDebugEnabled(bool enabled) {
    s_debugEnabled = enabled;
}

void Logger::LogSession(const std::filesystem::path& logPath, const std::string& message) {
    std::lock_guard<std::mutex> lock(s_logMutex);
    
    // Ensure directory exists
    if (logPath.has_parent_path()) {
        std::filesystem::create_directories(logPath.parent_path());
    }

    std::ofstream logFile(logPath, std::ios::app);
    if (logFile.is_open()) {
        logFile << "----> " << message << "\n";
    }
}

void Logger::LogTransfer(const std::filesystem::path& logPath, 
                        const std::string& fileName, 
                        const std::filesystem::path& sourceDir, 
                        const std::filesystem::path& destDir, 
                        const std::string& timestamp, 
                        const std::string& result) {
    std::lock_guard<std::mutex> lock(s_logMutex);

    // Ensure directory exists
    if (logPath.has_parent_path()) {
        std::filesystem::create_directories(logPath.parent_path());
    }

    std::ofstream logFile(logPath, std::ios::app);
    if (logFile.is_open()) {
        // CSV Format: "File Name","Source Directory","Destination Directory","Date and time of the completed operation","Success" or reason for failure
        logFile << "\"" << fileName << "\",\"" 
                << sourceDir.string() << "\",\"" 
                << destDir.string() << "\",\"" 
                << timestamp << "\",\"" 
                << result << "\"\n";

        if (s_debugEnabled) {
            std::cout << "Transfer: " << fileName << " -> " << destDir.string() << " [" << result << "]" << std::endl;
        }
    }


    // Log file trimming (if > 60,000 bytes, delete lines from the top until < 50,000 bytes)
    if (std::filesystem::exists(logPath) && std::filesystem::file_size(logPath) > 60000) {
        std::ifstream inFile(logPath);
        if (inFile.is_open()) {
            std::vector<std::string> lines;
            std::string line;
            while (std::getline(inFile, line)) {
                lines.push_back(line);
            }
            inFile.close();

            size_t currentSize = 0;
            size_t startIdx = 0;
            for (size_t i = 0; i < lines.size(); ++i) {
                currentSize += lines[i].length() + 1; // +1 for newline
                if (currentSize >= 50000) {
                    startIdx = i;
                    break;
                }
            }

            if (startIdx > 0) {
                std::ofstream outFile(logPath, std::ios::trunc);
                if (outFile.is_open()) {
                    for (size_t i = startIdx; i < lines.size(); ++i) {
                        outFile << lines[i] << "\n";
                    }
                }
            }
        }
    }
}

} // namespace FileMove
