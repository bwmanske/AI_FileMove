#include "group_dialog.hpp"
#include <string>
#include <vector>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#include <commctrl.h>
#include <gdiplus.h>
#include <shlobj.h>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")

using namespace Gdiplus;

namespace FileMove {

static std::wstring ToWString(const std::string& s) {
    if (s.empty()) return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, NULL, 0);
    std::wstring w(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &w[0], len);
    return w;
}

static std::string ToString(const wchar_t* ws) {
    if (!ws) return "";
    int len = WideCharToMultiByte(CP_UTF8, 0, ws, -1, NULL, 0, NULL, NULL);
    std::string s(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, ws, -1, &s[0], len, NULL, NULL);
    return s;
}

GroupDialog::GroupDialog(HWND hParent, GroupEditData& resultData)
    : m_hwndParent(hParent), m_hwndDialog(nullptr), m_hNameEdit(nullptr), 
      m_hDestListView(nullptr), m_hStatusLabel(nullptr), m_bSaved(false), m_resultData(resultData) {}

GroupDialog::~GroupDialog() {
    if (m_hImageList) ImageList_Delete(m_hImageList);
    if (m_gdiToken) GdiplusShutdown(m_gdiToken);
}

bool GroupDialog::ShowModal(const GroupEditData& initialData) {
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = GroupDialog::WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"GroupDialogWindowClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);

    if (!RegisterClassExW(&wc)) return false;

    m_hwndDialog = CreateWindowExW(
        0, L"GroupDialogWindowClass", L"Group Editor",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, 450, 450,
        m_hwndParent, NULL, wc.hInstance, this
    );

    if (!m_hwndDialog) return false;

    CreateControls();

    // Pre-populate
    SetWindowTextW(m_hNameEdit, ToWString(initialData.name).c_str());
    for (const auto& dest : initialData.destinations) {
        int idx = ListView_GetItemCount(m_hDestListView);
        LVITEMW lvi = {0};
        lvi.mask = LVIF_TEXT;
        lvi.iItem = idx;
        std::wstring destW = dest.wstring();
        lvi.pszText = (LPWSTR)destW.c_str();
        ListView_InsertItem(m_hDestListView, &lvi);
    }
    ValidateDestinations();

    ShowWindow(m_hwndDialog, SW_SHOW);
    UpdateWindow(m_hwndDialog);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (m_bSaved) {
        wchar_t nameBuffer[256];
        GetWindowTextW(m_hNameEdit, nameBuffer, 256);
        m_resultData.name = ToString(nameBuffer);
        m_resultData.destinations.clear();
        int count = ListView_GetItemCount(m_hDestListView);
        for (int i = 0; i < count; ++i) {
            wchar_t destBuf[MAX_PATH];
            ListView_GetItemTextW(m_hDestListView, i, 0, destBuf, MAX_PATH);
            m_resultData.destinations.push_back(std::filesystem::path(destBuf));
        }
        return true;
    }

    return false;
}

void GroupDialog::CreateControls() {
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&m_gdiToken, &gdiplusStartupInput, NULL);

    CreateWindowExW(0, L"STATIC", L"Group Name:", WS_CHILD | WS_VISIBLE, 10, 15, 100, 20, m_hwndDialog, NULL, GetModuleHandle(NULL), NULL);
    m_hNameEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE, 110, 12, 320, 25, m_hwndDialog, NULL, GetModuleHandle(NULL), NULL);

    CreateWindowExW(0, L"STATIC", L"Destinations:", WS_CHILD | WS_VISIBLE, 10, 50, 100, 20, m_hwndDialog, NULL, GetModuleHandle(NULL), NULL);
    m_hDestListView = CreateWindowExW(0, WC_LISTVIEW, L"", WS_CHILD | WS_VISIBLE | LVS_REPORT | WS_BORDER, 10, 75, 420, 260, m_hwndDialog, NULL, GetModuleHandle(NULL), NULL);

    LVCOLUMNW lvc = {0};
    lvc.mask = LVCF_TEXT | LVCF_WIDTH;
    lvc.pszText = (LPWSTR)L"Path";
    lvc.cx = 400;
    ListView_InsertColumn(m_hDestListView, 0, &lvc);

    m_hStatusLabel = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE, 10, 345, 420, 20, m_hwndDialog, NULL, GetModuleHandle(NULL), NULL);

    CreateWindowExW(0, L"BUTTON", L"Add Destination", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 375, 140, 30, m_hwndDialog, (HMENU)10, GetModuleHandle(NULL), NULL);
    CreateWindowExW(0, L"BUTTON", L"Remove Selected", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 160, 375, 140, 30, m_hwndDialog, (HMENU)11, GetModuleHandle(NULL), NULL);
    CreateWindowExW(0, L"BUTTON", L"Save", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 310, 375, 120, 30, m_hwndDialog, (HMENU)1, GetModuleHandle(NULL), NULL);
    CreateWindowExW(0, L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 180, 375, 120, 30, m_hwndDialog, (HMENU)2, GetModuleHandle(NULL), NULL);

    m_hImageList = ImageList_Create(32, 32, ILC_COLOR32 | ILC_MASK);
}

void GroupDialog::ValidateDestinations() {
    int count = ListView_GetItemCount(m_hDestListView);
    for (int i = 0; i < count; ++i) {
        wchar_t buf[MAX_PATH];
        ListView_GetItemTextW(m_hDestListView, i, 0, buf, MAX_PATH);
        std::filesystem::path p(buf);
        
        int iconIdx = 1; // Default red-X (missing)
        std::error_code ec;
        if (std::filesystem::exists(p, ec)) {
            iconIdx = 0; // Green check
        } else if (ec) {
            iconIdx = 2; // Orange question
        }

        LVITEMW lvi = {0};
        lvi.mask = LVIF_IMAGE;
        lvi.iItem = i;
        lvi.iImage = iconIdx;
        ListView_SetItem(m_hDestListView, &lvi);
    }
}

LRESULT CALLBACK GroupDialog::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    GroupDialog* pThis = nullptr;
    if (uMsg == WM_NCCREATE) {
        CREATESTRUCTW* pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
        pThis = reinterpret_cast<GroupDialog*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
        pThis = reinterpret_cast<GroupDialog*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (pThis) {
        switch (uMsg) {
            case WM_COMMAND: {
                int wmId = LOWORD(wParam);
                if (wmId == 1) { // Save
                    pThis->m_bSaved = true;
                    PostQuitMessage(0);
                } else if (wmId == 2) { // Cancel
                    pThis->m_bSaved = false;
                    PostQuitMessage(0);
                } else if (wmId == 10) { // Add
                    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
                    IFileOpenDialog* pfd = NULL;
                    if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, (void**)&pfd))) {
                        DWORD opts;
                        pfd->GetOptions(&opts);
                        pfd->SetOptions(opts | FOS_PICKFOLDERS);
                        if (SUCCEEDED(pfd->Show(pThis->m_hwndDialog))) {
                            IShellItem* pItem = NULL;
                            if (SUCCEEDED(pfd->GetResult(&pItem))) {
                                PWSTR pszPath = NULL;
                                if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszPath))) {
                                    std::wstring pathW(pszPath);
                                    LVITEMW lvi = {0};
                                    lvi.mask = LVIF_TEXT;
                                    int idx = ListView_GetItemCount(pThis->m_hDestListView);
                                    lvi.iItem = idx;
                                    lvi.pszText = (LPWSTR)pathW.c_str();
                                    ListView_InsertItem(pThis->m_hDestListView, &lvi);
                                    CoTaskMemFree(pszPath);
                                }
                                pItem->Release();
                            }
                        }
                        pfd->Release();
                    }
                    CoUninitialize();
                } else if (wmId == 11) { // Remove
                    int idx = ListView_GetNextItem(pThis->m_hDestListView, -1, LVNI_SELECTED);
                    if (idx != -1) {
                        ListView_DeleteItem(pThis->m_hDestListView, idx);
                        pThis->ValidateDestinations();
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
}

} // namespace FileMove
#endif
