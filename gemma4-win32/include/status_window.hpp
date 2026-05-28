#ifndef STATUS_WINDOW_HPP
#define STATUS_WINDOW_HPP

#include <string>
#include <vector>
#include "filemove.hpp"

#ifdef _WIN32
#include <windows.h>
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

class MainWindow; // Forward declaration

class StatusWindow {
public:
    StatusWindow(HWND hParent, Settings& currentSettings, MoveWorker& worker, MainWindow* pMainWindow);
    ~StatusWindow();

    void ShowModal();

private:
#ifdef _WIN32
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void CreateControls();
    void UpdateUI();
    void OnOpenLog();
    void OnSelectJson(const std::string& jsonPath);
    void OnNewJson();
    void OnPauseResume();

    HWND m_hwndParent;
    HWND m_hwndDialog = nullptr;
    Settings& m_settings;
    MoveWorker& m_worker;
    MainWindow* m_pMainWindow;

    // UI Elements
    HWND m_hActiveJsonLabel = nullptr;
    HWND m_hActiveLogLabel = nullptr;
    HWND m_hJsonListView = nullptr;
    HWND m_hQueueStatusLabel = nullptr;
    HWND m_hWorkerStateLabel = nullptr;
    HWND m_hPauseResumeButton = nullptr;
#endif
};

} // namespace FileMove

#endif // STATUS_WINDOW_HPP
