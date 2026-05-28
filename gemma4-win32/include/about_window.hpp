#ifndef ABOUT_WINDOW_HPP
#define ABOUT_WINDOW_HPP

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

class AboutWindow {
public:
    AboutWindow(HWND hParent, const std::string& cmdLine);
    ~AboutWindow();

    void ShowModal();

private:
#ifdef _WIN32
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    HWND m_hwndParent;
    HWND m_hwndDialog;
    std::string m_cmdLine;

    // Build info components
    HWND m_hVersionLabel;     // Left justified
    HWND m_hBuiltOnLabel;     // Right justified
    HWND m_hCommandLineLabel; // Full line below
#else
    HWND m_hwndParent;
    std::string m_cmdLine;
#endif
};

} // namespace FileMove

#endif // ABOUT_WINDOW_HPP
