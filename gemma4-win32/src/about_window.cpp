#include "about_window.hpp"
#include <string>
#include <sstream>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#include <gdiplus.h>

#pragma comment(lib, "gdiplus.lib")

namespace FileMove {

using namespace Gdiplus;

// Helper to convert wstring to string (needed for groupName extraction)
static std::string ToString(const wchar_t* ws) {
    if (!ws) return "";
    std::wstring w(ws);
    return std::string(w.begin(), w.end());
}

// Helper to convert string to wstring
static std::wstring ToWString(const std::string& s) {
    return std::wstring(s.begin(), s.end());
}

AboutWindow::AboutWindow(HWND hParent, const std::string& cmdLine)
    : m_hwndParent(hParent), m_hwndDialog(NULL), m_cmdLine(cmdLine) {}

AboutWindow::~AboutWindow() {}

void AboutWindow::ShowModal() {
#ifdef _WIN32
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = AboutWindow::WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"AboutWindowClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if (!RegisterClassExW(&wc)) {
        GdiplusShutdown(gdiplusToken);
        return;
    }

    m_hwndDialog = CreateWindowExW(
        WS_EX_DLGMODALFRAME,
        L"AboutWindowClass",
        L"About",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 350,
        m_hwndParent, NULL, wc.hInstance, this
    );

    if (m_hwndDialog) {
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    GdiplusShutdown(gdiplusToken);
#endif
}

LRESULT CALLBACK AboutWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
#ifdef _WIN32
    AboutWindow* pThis = nullptr;

    if (uMsg == WM_NCCREATE) {
        CREATESTRUCTW* pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
        pThis = reinterpret_cast<AboutWindow*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
        pThis = reinterpret_cast<AboutWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (pThis) {
        switch (uMsg) {
            case WM_CREATE: {
                // Create labels for Build Information
                pThis->m_hVersionLabel = CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE, 
                    20, 180, 150, 20, hwnd, NULL, NULL, NULL);
                pThis->m_hBuiltOnLabel = CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE, 
                    230, 180, 150, 20, hwnd, NULL, NULL, NULL);
                pThis->m_hCommandLineLabel = CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE, 
                    20, 205, 360, 40, hwnd, NULL, NULL, NULL);

                // Set text content
                std::string versionStr = "Version: " + std::string(FILEMOVE_VERSION);
                // We'll need a way to get build time. For now using placeholder or if defined.
                // Since I don't see FILEMOVE_BUILD_TIME in the header, checking if it exists via macros is safer.
                std::string builtOnStr = "Built On: Unknown"; 
                #ifdef FILEMOVE_BUILD_TIME
                builtOnStr = "Built On: " + std::string(FILEMOVE_BUILD_TIME);
                #endif

                std::string cmdLineStr = "Command Line: " + pThis->m_cmdLine;

                SendMessageW(pThis->m_hVersionLabel, WM_SETTEXT, 0, (LPARAM)ToWString(versionStr).c_str());
                SendMessageW(pThis->m_hBuiltOnLabel, WM_SETTEXT, 0, (LPARAM)ToWString(builtOnStr).c_str());
                SendMessageW(pThis->m_hCommandLineLabel, WM_SETTEXT, 0, (LPARAM)ToWString(cmdLineStr).c_str());

                return 0;
            }
            case WM_PAINT: {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                {
                    Graphics graphics(hdc);
                    // Load the image. Note: In a real app we'd use an absolute path or resource ID.
                    // Using relative path based on current working directory/project structure.
                    Image image(L"gemma4-win32/resources/images/about-image.png");

                    if (image.GetLastStatus() == Ok) {
                        int imgWidth = image.GetWidth();
                        int imgHeight = image.GetHeight();
                        // Center the 128x128 image at top
                        int x = (GetClientRect(hwnd).right - imgWidth) / 2;
                        int y = 20;
                        graphics.DrawImage(&image, x, y, imgWidth, imgHeight);
                    }
                }
                EndPaint(hwnd, &ps);
                return 0;
            }
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
        }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
#else
    return 0;
#endif
}

} // namespace FileMove
#endif
