#ifndef SETTINGS_WINDOW_HPP
#define SETTINGS_WINDOW_HPP

#include <string>
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

class SettingsWindow {
public:
    SettingsWindow(HWND hParent, const Settings& currentSettings);
    ~SettingsWindow();

    void ShowModal();

private:
#ifdef _WIN32
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    HWND m_hwndParent;
    HWND m_hwndDialog;
    Settings& m_settings;

    // UI Elements
    HWND m_hSortRadioGroup = nullptr;
    HWND m_hPlacementRadioGroup = nullptr;
    HWND m_hWidthEdit = nullptr;
    HWND m_hHeightEdit = nullptr;
    HWND m_hLeftEdit = nullptr;
    HWND m_hTopEdit = nullptr;

    // v1.1.0 Options
    HWND m_hCheckDirMoves = nullptr;
    HWND m_hCheckSidecars = nullptr;
    HWND m_hCheckHiddenFiles = nullptr;
#endif
};

} // namespace FileMove

#endif // SETTINGS_WINDOW_HPP
