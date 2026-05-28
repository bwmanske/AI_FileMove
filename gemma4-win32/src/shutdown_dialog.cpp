#include "shutdown_dialog.hpp"

#ifdef _WIN32
#include <windows.h>
#include <commctrl.h>

namespace FileMove {

enum ControlId {
    ID_BTN_CANCEL = 1,
    ID_BTN_FINISH = 2,
    ID_BTN_IMMEDIATE = 3
};

ShutdownDialog::ShutdownDialog(HWND hParent)
    : m_hwndParent(hParent), m_hwndDialog(nullptr) {}

ShutdownDialog::~ShutdownDialog() {}

ShutdownResult ShutdownDialog::ShowModal() {
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = ShutdownDialog::WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"ShutdownDialogWindowClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);

    if (!RegisterClassExW(&wc)) {
        return ShutdownResult::Cancel;
    }

    m_hwndDialog = CreateWindowExW(
        WS_EX_DLGMODALFRAME,
        L"ShutdownDialogWindowClass",
        L"Shutdown Prompt",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 350, 180,
        m_hwndParent, NULL, wc.hInstance, this
    );

    if (!m_hwndDialog) {
        return ShutdownResult::Cancel;
    }

    // Create controls
    CreateWindowW(L"STATIC", L"Work is in progress or queued.\nHow would you like to proceed?", 
                 WS_CHILD | WS_VISIBLE, 20, 20, 310, 40, m_hwndDialog, NULL, GetModuleHandle(NULL), NULL);

    CreateWindowW(L"BUTTON", L"Cancel Shutdown", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 
                 20, 70, 100, 30, m_hwndDialog, (HMENU)ID_BTN_CANCEL, GetModuleHandle(NULL), NULL);

    CreateWindowW(L"BUTTON", L"Finish Current File And Exit", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 
                 125, 70, 180, 30, m_hwndDialog, (HMENU)ID_BTN_FINISH, GetModuleHandle(NULL), NULL);

    CreateWindowW(L"BUTTON", L"Cancel Active File And Exit Immediately", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 
                 20, 110, 285, 30, m_hwndDialog, (HMENU)ID_BTN_IMMEDIATE, GetModuleHandle(NULL), NULL);

    MSG msg;
    ShutdownResult result = ShutdownResult::Cancel;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (msg.message == WM_COMMAND) {
            int wmId = LOWORD(msg.wParam);
            if (wmId == ID_BTN_CANCEL) result = ShutdownResult::Cancel;
            else if (wmId == ID_BTN_FINISH) result = ShutdownResult::FinishCurrent;
            else if (wmId == ID_BTN_IMMEDIATE) result = ShutdownResult::Immediate;
            
            if (result != ShutdownResult::Cancel) {
                PostQuitMessage(0);
            }
        }
    }

    return result;
}

LRESULT CALLBACK ShutdownDialog::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    ShutdownDialog* pThis = nullptr;
    if (uMsg == WM_NCCREATE) {
        CREATESTRUCTW* pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
        pThis = reinterpret_cast<ShutdownDialog*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
        pThis = reinterpret_cast<ShutdownDialog*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (pThis) {
        if (uMsg == WM_DESTROY) {
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

} // namespace FileMove
#endif
