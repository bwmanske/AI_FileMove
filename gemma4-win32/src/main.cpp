#include "filemove.hpp"
#include "mainwindow.hpp"
#include "logger.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>

static std::wstring ToWString(const std::string& s) {
    return std::wstring(s.begin(), s.end());
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    int argc;
    LPWSTR* argvW = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argvW) return 1;

    // Convert WCHAR array to char* array for our parser
    std::vector<char*> argv;
    for (int i = 0; i < argc; ++i) {
        int size = WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, nullptr, 0, nullptr, nullptr);
        char* arg = new char[size];
        WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, arg, size, nullptr, nullptr);
        argv.push_back(arg);
    }

    // Prepare log path early for error logging
    std::filesystem::path dataDir = FileMove::StorageManager::getDefaultDataDirectory();
    std::filesystem::path logPath = dataDir / "FileMove.log";

    CommandLineOptions options;
    try {
        options = CommandLineParser::parse(argc, argv.data());
    } catch (const ParseException& e) {
        // Log the error before exiting
        std::string errMsg = "Command-line parsing error: " + std::string(e.what());
        FileMove::Logger::LogSession(logPath, errMsg);
        FileMove::Logger::LogError(errMsg);
        MessageBoxW(NULL, ToWString(e.what()).c_str(), L"Parse Error", MB_OK | MB_ICONERROR);
        for (auto arg : argv) delete[] arg;
        LocalFree(argvW);
        return 1;
    }

    FileMove::MainWindow app(hInstance, options);
    
    // Log App Start
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
    std::string startTime = ss.str();
    
    FileMove::Logger::LogSession(logPath, "App started: " + startTime);
    FileMove::Logger::LogDebug("App started: " + startTime);
    FileMove::Logger::LogDebug("Command line options: " + std::string(lpCmdLine));
    FileMove::Logger::LogSession(logPath, "Active sort mode: " + std::to_string(static_cast<int>(options.sortMode)));
    FileMove::Logger::LogSession(logPath, "Active placement mode: " + std::to_string(static_cast<int>(options.placementMode)));

    if (app.Create(nShowCmd)) {
        app.RunMessageLoop();
    } else {
        for (auto arg : argv) delete[] arg;
        LocalFree(argvW);
        return 0;
    }

    // Log App Exit/Close
    FileMove::Logger::LogSession(logPath, "App closing");
    FileMove::Logger::LogDebug("App closing");

    for (auto arg : argv) delete[] arg;
    LocalFree(argvW);
    return 0;
}
#else
#include "storage_manager.hpp"

int main(int argc, char* argv[]) {
    std::cout << "FileMove (Linux mode) starting..." << std::endl;

    FileMove::CommandLineOptions options;
    try {
        options = FileMove::CommandLineParser::parse(argc, argv);
    } catch (const FileMove::ParseException& e) {
        std::cerr << "Command-line parsing error: " << e.what() << std::endl;
        return 1;
    }

    // Load settings from the provided JSON file if it exists
    FileMove::Settings settings;
    if (!options.inputJsonPath.empty()) {
        try {
            settings = FileMove::StorageManager::loadSettings(options.inputJsonPath);
        } catch (const std::exception& e) {
            std::cerr << "Error loading settings: " << e.what() << std::endl;
            return 1;
        }
    }

    // Use the provided output log path if available, otherwise default
    std::filesystem::path logPath = options.outputLogPath.empty() ? 
                                    FileMove::StorageManager::getDefaultDataDirectory() / "FileMove.log" : 
                                    options.outputLogPath;

    FileMove::MoveWorker worker;
    worker.Start();

    if (!options.testFiles.empty()) {
        std::cout << "Headless mode: Processing " << options.testFiles.size() << " files..." << std::endl;
        
        // Collect all raw entries first
        std::vector<FileMove::PendingMoveEntry> rawEntries;
        for (const auto& filePath : options.testFiles) {
            if (!std::filesystem::exists(filePath)) {
                std::cerr << "Warning: Test file does not exist: " << filePath << std::endl;
                continue;
            }

            if (settings.groups.empty()) {
                std::cerr << "Error: No groups defined in settings to move files into." << std::endl;
                break;
            }

            const auto& targetGroup = settings.groups[0];
            if (targetGroup.destinationPaths.empty()) {
                std::cerr << "Error: Group '" << targetGroup.name << "' has no destinations." << std::endl;
                continue;
            }

            FileMove::PendingMoveEntry entry;
            entry.id = "test-" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
            entry.groupId = targetGroup.id;
            entry.sourceFilePath = filePath;
            entry.relativePath = filePath.filename();
            entry.destinationDirectories = targetGroup.destinationPaths;
            entry.debugTransferMode = options.debugMode == FileMove::DebugMode::MV ? FileMove::DebugMode::MV : 
                                     (options.debugMode == FileMove::DebugMode::CP ? FileMove::DebugMode::CP : FileMove::DebugMode::None);
            entry.queuedAt = "test-time";
            entry.status = FileMove::MoveStatus::Queued;

            rawEntries.push_back(entry);
        }

         // Now expand them (this handles directory expansion and sidecars)
         auto preparedBatch = worker.PrepareBatch(rawEntries);
         size_t targetCount = 0;
         if (preparedBatch && !preparedBatch->empty()) {
             targetCount = preparedBatch->size();
             for (auto& entry : *preparedBatch) {
                 entry.activeLogFilePath = logPath;
             }
             worker.QueueBatch(*preparedBatch);
         } else if (!rawEntries.empty()) {
              std::cerr << "Error: Preparation failed for the batch." << std::endl;
              targetCount = rawEntries.size(); // Fallback to raw count to avoid infinite loop
         }


        // Wait for processing to complete or timeout (30s)
        int attempts = 0;
        while ((worker.GetQueuedCount() > 0 || worker.GetProcessedCount() < targetCount) && attempts < 60) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            attempts++;
        }

        if (attempts >= 60) {
            std::cerr << "Timeout reached while waiting for worker." << std::endl;
        } else {
             std::cout << "Processing finished. Processed: " << worker.GetProcessedCount() << "/" << targetCount << std::endl;
        }
    } else {
        std::cout << "No test files provided. Nothing to do in headless mode." << std::endl;
    }

    worker.Stop();
    return 0;
}
#endif
