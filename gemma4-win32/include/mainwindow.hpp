#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <string>
#include <vector>
#include <filesystem>
#include "filemove.hpp"

#ifdef _WIN32
#include <windows.h>
#include <commctrl.h>
#else
// Dummy types for Linux compilation/testing
typedef void* HWND;
typedef void* HINSTANCE;
typedef void* HANDLE;
typedef void* LPVOID;
typedef void* LPSTR;
typedef void* LPWSTR;
typedef void* LPARAM;
typedef void* WPARAM;
typedef unsigned int UINT;
typedef long long LRESULT;
#define CALLBACK 
#endif

namespace FileMove {

class SettingsWindow; // Forward declaration
class StatusWindow;  // Forward declaration
class QueueWindow;   // Forward declaration

class MainWindow {
public:
    MainWindow(HINSTANCE hInstance, const CommandLineOptions& options);
    ~MainWindow();

    bool Create(int nCmdShow);
    void RunMessageLoop();

    // Getters for other windows
    std::filesystem::path GetActiveJsonPath() const { return m_activeJsonPath; }
    std::filesystem::path GetActiveLogPath() const { return m_activeLogPath; }
    void UpdateStatusLabels(const std::string& jsonPath, const std::string& logPath);

private:
#ifdef _WIN32
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

#ifdef _WIN32
    HINSTANCE m_hInstance;
    HWND m_hwnd;
    std::string m_cmdLine;

    // UI Elements
    HWND m_hListView = nullptr;
    HWND m_hStatusLabelLeft = nullptr;
    HWND m_hStatusLabelRight = nullptr;
    HWND m_hTooltip = nullptr;

    std::filesystem::path m_activeJsonPath;
    std::filesystem::path m_activeLogPath;

    MoveWorker m_worker;
    Settings m_settings;
    CommandLineOptions m_options;

    // Windows
    // Note: In a real app, these might be managed differently. 
    // For now we'll use members for simplicity.
    // StatusWindow is currently used modally in src/mainwindow.cpp (Wait, it uses its own loop).
    // QueueWindow is non-modal.
    QueueWindow m_queueWindow;

    void InitializeControls();
    Settings& GetSettings() { return m_settings; }
    MoveWorker& GetWorker() { return m_worker; }
#else
    HINSTANCE m_hInstance;
    HWND m_hwnd;
    std::string m_cmdLine;
    HWND m_hListView = nullptr;
    HWND m_hStatusLabelLeft = nullptr;
    HWND m_hStatusLabelRight = nullptr;
    std::filesystem::path m_activeJsonPath;
    std::filesystem::path m_activeLogPath;
    MoveWorker m_worker;
    Settings m_settings;
    CommandLineOptions m_options;
#endif
};

} // namespace FileMove

#endif // MAINWINDOW_HPP
