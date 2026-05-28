#include "search_dialog.hpp"
#include <string>

#ifdef _WIN32
#include <commctrl.h>

namespace FileMove {

static std::string ToString(const wchar_t* ws) {
    if (!ws) return "";
    std::wstring w(ws);
    return std::string(w.begin(), w.end());
}

SearchDialog::SearchDialog(HWND hParent) 
    : m_hwndParent(hParent), m_hwndDialog(nullptr), m_hEdit(nullptr), m_result(""), m_bSuccess(false) {}

SearchDialog::~SearchDialog() {}

std::string SearchDialog::ShowModal() {
    const wchar_t CLASS_NAME[] = L"SearchDialogWindowClass";

    WNDCLASSW wc = {};
    wc.lpfnWndProc   = SearchDialog::WindowProc;
    wc.hInstance     = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);

    RegisterClassW(&wc);

    m_hwndDialog = CreateWindowExW(
        0,
        CLASS_NAME,
        L"Search Groups",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, 300, 150,
        m_hwndParent,
        NULL,
        GetModuleHandle(NULL),
        this
    );

    if (m_hwndDialog == NULL) {
        return "";
    }

    // Create controls
    CreateWindowExW(0, L"STATIC", L"Search:", WS_CHILD | WS_VISIBLE, 10, 20, 50, 20, m_hwndDialog, NULL, GetModuleHandle(NULL), NULL);
    m_hEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 70, 17, 210, 25, m_hwndDialog, NULL, GetModuleHandle(NULL), NULL);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"BUTTON", L"OK", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 100, 70, 80, 30, m_hwndDialog, (HMENU)1, GetModuleHandle(NULL), NULL);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 190, 70, 80, 30, m_hwndDialog, (HMENU)2, GetModuleHandle(NULL), NULL);

    ShowWindow(m_hwndDialog, SW_SHOW);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (m_bSuccess) {
        return m_result;
    }

    return "";
}

LRESULT CALLBACK SearchDialog::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    SearchDialog* pThis = nullptr;

    if (uMsg == WM_NCCREATE) {
        CREATESTRUCTW* pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
        pThis = reinterpret_cast<SearchDialog*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
        pThis = reinterpret_cast<SearchDialog*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (pThis) {
        switch (uMsg) {
            case WM_COMMAND: {
                int wmId = LOWORD(wParam);
                if (wmId == 1) { // OK
                    wchar_t buffer[256];
                    GetWindowTextW(pThis->m_hEdit, buffer, 256);
                    pThis->m_result = ToString(buffer);
                    pThis->m_bSuccess = true;
                    PostQuitMessage(0);
                } else if (wmId == 2) { // Cancel
                    pThis->m_bSuccess = false;
                    PostQuitMessage(0);
                }
                return 0;
            }
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
            default:
                return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

} // namespace FileMove
#endif

