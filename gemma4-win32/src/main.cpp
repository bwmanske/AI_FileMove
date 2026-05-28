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
int main(int argc, char* argv[]) {
    std::cout << "FileMove (Linux/Wine mode) starting..." << std::endl;
    return 0;
}
#endif
