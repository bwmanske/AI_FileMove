#include "settings_window.hpp"
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <commctrl.h>

namespace FileMove {

enum ControlId {
    ID_SORT_MRU = 100,
    ID_SORT_LRU,
    ID_SORT_AF,
    ID_SORT_AL,
    ID_SORT_AZ,
    ID_SORT_ZA,
    ID_PLACE_UL = 200,
    ID_PLACE_UR,
    ID_PLACE_LL,
    ID_PLACE_LR,
    ID_PLACE_LAST,
    ID_BTN_SAVE = 300,
    ID_BTN_CANCEL = 301,

    ID_CHECK_DIR_MOVES = 400,
    ID_CHECK_SIDECARS,
    ID_CHECK_HIDDEN_FILES
};

SettingsWindow::SettingsWindow(HWND hParent, const Settings& currentSettings)
    : m_hwndParent(hParent), m_hwndDialog(nullptr), m_settings(const_cast<Settings&>(currentSettings)) {}

SettingsWindow::~SettingsWindow() {}

void SettingsWindow::ShowModal() {
#ifdef _WIN32
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = SettingsWindow::WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"SettingsWindowClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);

    if (!RegisterClassExW(&wc)) {
        return;
    }

     m_hwndDialog = CreateWindowExW(
         WS_EX_DLGMODALFRAME,
         L"SettingsWindowClass",
         L"Settings",
         WS_OVERLAPPEDWINDOW | WS_VISIBLE,
         CW_USEDEFAULT, CW_USEDEFAULT, 350, 620,
         m_hwndParent, NULL, wc.hInstance, this
     );

    if (m_hwndDialog) {
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
#endif
}

LRESULT CALLBACK SettingsWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
#ifdef _WIN32
    SettingsWindow* pThis = nullptr;

    if (uMsg == WM_NCCREATE) {
        CREATESTRUCTW* pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
        pThis = reinterpret_cast<SettingsWindow*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
        pThis = reinterpret_cast<SettingsWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (pThis) {
        switch (uMsg) {
            case WM_CREATE: {
                // Sort Order Group
                CreateWindowExW(0, L"STATIC", L"Sort Order:", WS_CHILD | WS_VISIBLE, 15, 15, 150, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
                
                CreateWindowExW(0, L"BUTTON", L"Most Recently Used", WS_CHILD | WS_VISIBLE | WS_GROUP | BS_AUTORADIOBUTTON, 20, 40, 180, 20, hwnd, (HMENU)ID_SORT_MRU, GetModuleHandle(NULL), NULL);
                CreateWindowExW(0, L"BUTTON", L"Least Recently Used", WS_CHILD | WS_VISIBLE | WS_GROUP | BS_AUTORADIOBUTTON, 20, 65, 180, 20, hwnd, (HMENU)ID_SORT_LRU, GetModuleHandle(NULL), NULL);
                CreateWindowExW(0, L"BUTTON", L"Added First", WS_CHILD | WS_VISIBLE | WS_GROUP | BS_AUTORADIOBUTTON, 20, 90, 180, 20, hwnd, (HMENU)ID_SORT_AF, GetModuleHandle(NULL), NULL);
                CreateWindowExW(0, L"BUTTON", L"Added Last", WS_CHILD | WS_VISIBLE | WS_GROUP | BS_AUTORADIOBUTTON, 20, 115, 180, 20, hwnd, (HMENU)ID_SORT_AL, GetModuleHandle(NULL), NULL);
                CreateWindowExW(0, L"BUTTON", L"A thru Z", WS_CHILD | WS_VISIBLE | WS_GROUP | BS_AUTORADIOBUTTON, 20, 140, 180, 20, hwnd, (HMENU)ID_SORT_AZ, GetModuleHandle(NULL), NULL);
                CreateWindowExW(0, L"BUTTON", L"Z thru A", WS_CHILD | WS_VISIBLE | WS_GROUP | BS_AUTORADIOBUTTON, 20, 165, 180, 20, hwnd, (HMENU)ID_SORT_ZA, GetModuleHandle(NULL), NULL);

                // Placement Group
                CreateWindowExW(0, L"STATIC", L"Placement:", WS_CHILD | WS_VISIBLE, 15, 200, 150, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
                
                CreateWindowExW(0, L"BUTTON", L"Upper Left", WS_CHILD | WS_VISIBLE | WS_GROUP | BS_AUTORADIOBUTTON, 20, 225, 180, 20, hwnd, (HMENU)ID_PLACE_UL, GetModuleHandle(NULL), NULL);
                CreateWindowExW(0, L"BUTTON", L"Upper Right", WS_CHILD | WS_VISIBLE | WS_GROUP | BS_AUTORADIOBUTTON, 20, 250, 180, 20, hwnd, (HMENU)ID_PLACE_UR, GetModuleHandle(NULL), NULL);
                CreateWindowExW(0, L"BUTTON", L"Lower Left", WS_CHILD | WS_VISIBLE | WS_GROUP | BS_AUTORADIOBUTTON, 20, 275, 180, 20, hwnd, (HMENU)ID_PLACE_LL, GetModuleHandle(NULL), NULL);
                CreateWindowExW(0, L"BUTTON", L"Lower Right", WS_CHILD | WS_VISIBLE | WS_GROUP | BS_AUTORADIOBUTTON, 20, 300, 180, 20, hwnd, (HMENU)ID_PLACE_LR, GetModuleHandle(NULL), NULL);
                CreateWindowExW(0, L"BUTTON", L"Last Location", WS_CHILD | WS_VISIBLE | WS_GROUP | BS_AUTORADIOBUTTON, 20, 325, 180, 20, hwnd, (HMENU)ID_PLACE_LAST, GetModuleHandle(NULL), NULL);

                // v1.1.0 Options
                CreateWindowExW(0, L"STATIC", L"Options:", WS_CHILD | WS_VISIBLE, 15, 360, 150, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
                m_hCheckDirMoves = CreateWindowExW(0, L"BUTTON", L"Enable directory moves", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 20, 385, 250, 20, hwnd, (HMENU)ID_CHECK_DIR_MOVES, GetModuleHandle(NULL), NULL);
                m_hCheckSidecars = CreateWindowExW(0, L"BUTTON", L"Enable sidecar files", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 20, 410, 250, 20, hwnd, (HMENU)ID_CHECK_SIDECARS, GetModuleHandle(NULL), NULL);
                m_hCheckHiddenFiles = CreateWindowExW(0, L"BUTTON", L"Hide queued source files", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 20, 435, 250, 20, hwnd, (HMENU)ID_CHECK_HIDDEN_FILES, GetModuleHandle(NULL), NULL);

                // Startup Preview
                CreateWindowExW(0, L"STATIC", L"Startup Preview:", WS_CHILD | WS_VISIBLE, 15, 470, 150, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);

                // Size: [ W ] [ H ]
                CreateWindowExW(0, L"STATIC", L"Size:", WS_CHILD | WS_VISIBLE, 20, 495, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
                m_hWidthEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", std::to_wstring(pThis->m_settings.windowWidth).c_str(), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_READONLY, 65, 495, 50, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
                CreateWindowExW(0, L"STATIC", L"W", WS_CHILD | WS_VISIBLE, 120, 495, 20, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);

                m_hHeightEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", std::to_wstring(pThis->m_settings.windowHeight).c_str(), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_READONLY, 145, 495, 50, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
                CreateWindowExW(0, L"STATIC", L"H", WS_CHILD | WS_VISIBLE, 200, 495, 20, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);

                // Saved Position: Left [ ] Top [ ]
                CreateWindowExW(0, L"STATIC", L"Saved Position:", WS_CHILD | WS_VISIBLE, 20, 525, 150, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);

                m_hLeftEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", std::to_wstring(pThis->m_settings.windowLeft).c_str(), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_READONLY, 140, 525, 50, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
                CreateWindowExW(0, L"STATIC", L"Left", WS_CHILD | WS_VISIBLE, 195, 525, 35, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);

                m_hTopEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", std::to_wstring(pThis->m_settings.windowTop).c_str(), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_READONLY, 240, 525, 50, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
                CreateWindowExW(0, L"STATIC", L"Top", WS_CHILD | WS_VISIBLE, 295, 525, 35, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);

                // Buttons
                CreateWindowExW(0, L"BUTTON", L"Save", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 170, 575, 80, 30, hwnd, (HMENU)ID_BTN_SAVE, GetModuleHandle(NULL), NULL);
                CreateWindowExW(0, L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 260, 575, 80, 30, hwnd, (HMENU)ID_BTN_CANCEL, GetModuleHandle(NULL), NULL);

                // Set initial state
                if (pThis->m_settings.sortMode == SortMode::MostRecentlyUsed) SendMessage(hwnd, BM_SETCHECK, (WPARAM)ID_SORT_MRU, (LPARAM)BST_CHECKED);
                else if (pThis->m_settings.sortMode == SortMode::LeastRecentlyUsed) SendMessage(hwnd, BM_SETCHECK, (WPARAM)ID_SORT_LRU, (LPARAM)BST_CHECKED);
                else if (pThis->m_settings.sortMode == SortMode::AddedFirst) SendMessage(hwnd, BM_SETCHECK, (WPARAM)ID_SORT_AF, (LPARAM)BST_CHECKED);
                else if (pThis->m_settings.sortMode == SortMode::AddedLast) SendMessage(hwnd, BM_SETCHECK, (WPARAM)ID_SORT_AL, (LPARAM)BST_CHECKED);
                else if (pThis->m_settings.sortMode == SortMode::AtoZ) SendMessage(hwnd, BM_SETCHECK, (WPARAM)ID_SORT_AZ, (LPARAM)BST_CHECKED);
                else if (pThis->m_settings.sortMode == SortMode::ZtoA) SendMessage(hwnd, BM_SETCHECK, (WPARAM)ID_SORT_ZA, (LPARAM)BST_CHECKED);

                if (pThis->m_settings.placementMode == PlacementMode::UpperLeft) SendMessage(hwnd, BM_SETCHECK, (WPARAM)ID_PLACE_UL, (LPARAM)BST_CHECKED);
                else if (pThis->m_settings.placementMode == PlacementMode::UpperRight) SendMessage(hwnd, BM_SETCHECK, (WPARAM)ID_PLACE_UR, (LPARAM)BST_CHECKED);
                else if (pThis->m_settings.placementMode == PlacementMode::LowerLeft) SendMessage(hwnd, BM_SETCHECK, (WPARAM)ID_PLACE_LL, (LPARAM)BST_CHECKED);
                else if (pThis->m_settings.placementMode == PlacementMode::LowerRight) SendMessage(hwnd, BM_SETCHECK, (WPARAM)ID_PLACE_LR, (LPARAM)BST_CHECKED);
                else if (pThis->m_settings.placementMode == PlacementMode::LastLocation) SendMessage(hwnd, BM_SETCHECK, (WPARAM)ID_PLACE_LAST, (LPARAM)BST_CHECKED);

                // Set checkbox states
                SendMessage(m_hCheckDirMoves, BM_SETCHECK, (WPARAM)pThis->m_settings.enableDirectoryMoves ? BST_CHECKED : BST_UNCHECKED, 0);
                SendMessage(m_hCheckSidecars, BM_SETCHECK, (WPARAM)pThis->m_settings.enableSidecarFiles ? BST_CHECKED : BST_UNCHECKED, 0);
                SendMessage(m_hCheckHiddenFiles, BM_SETCHECK, (WPARAM)pThis->m_settings.hideQueuedSourceFiles ? BST_CHECKED : BST_UNCHECKED, 0);

                return 0;
            }
            }
            case WM_COMMAND: {
                int wmId = LOWORD(wParam);
                if (wmId == ID_BTN_SAVE) {
                    // Update all settings from UI before exiting
                    if (wmId >= ID_SORT_MRU && wmId <= ID_SORT_ZA) { /* already done in WM_COMMAND check for radio buttons */ }
                    else if (wmId >= ID_PLACE_UL && wmId <= ID_PLACE_LAST) { /* already done */ }

                    if (SendMessage(pThis->m_hCheckDirMoves, BM_GETCHECK, 0, 0) == BST_CHECKED) pThis->m_settings.enableDirectoryMoves = true;
                    else pThis->m_settings.enableDirectoryMoves = false;

                    if (SendMessage(pThis->m_hCheckSidecars, BM_GETCHECK, 0, 0) == BST_CHECKED) pThis->m_settings.enableSidecarFiles = true;
                    else pThis->m_settings.enableSidecarFiles = false;

                    if (SendMessage(pThis->m_hCheckHiddenFiles, BM_GETCHECK, 0, 0) == BST_CHECKED) pThis->m_settings.hideQueuedSourceFiles = true;
                    else pThis->m_settings.hideQueuedSourceFiles = false;

                    PostQuitMessage(0);
                } else if (wmId == ID_BTN_CANCEL) {
                    PostQuitMessage(0);
                } else if ((wmId >= ID_SORT_MRU && wmId <= ID_SORT_ZA) || 
                           (wmId >= ID_PLACE_UL && wmId <= ID_PLACE_LAST)) {
                    if (wmId >= ID_SORT_MRU && wmId <= ID_SORT_ZA) {
                        if (wmId == ID_SORT_MRU) pThis->m_settings.sortMode = SortMode::MostRecentlyUsed;
                        else if (wmId == ID_SORT_LRU) pThis->m_settings.sortMode = SortMode::LeastRecentlyUsed;
                        else if (wmId == ID_SORT_AF) pThis->m_settings.sortMode = SortMode::AddedFirst;
                        else if (wmId == ID_SORT_AL) pThis->m_settings.sortMode = SortMode::AddedLast;
                        else if (wmId == ID_SORT_AZ) pThis->m_settings.sortMode = SortMode::AtoZ;
                        else if (wmId == ID_SORT_ZA) pThis->m_settings.sortMode = SortMode::ZtoA;
                    } else {
                        if (wmId == ID_PLACE_UL) pThis->m_settings.placementMode = PlacementMode::UpperLeft;
                        else if (wmId == ID_PLACE_UR) pThis->m_settings.placementMode = PlacementMode::UpperRight;
                        else if (wmId == ID_PLACE_LL) pThis->m_settings.placementMode = PlacementMode::LowerLeft;
                        else if (wmId == ID_PLACE_LR) pThis->m_settings.placementMode = PlacementMode::LowerRight;
                        else if (wmId == ID_PLACE_LAST) pThis->m_settings.placementMode = PlacementMode::LastLocation;
                    }
                }
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
