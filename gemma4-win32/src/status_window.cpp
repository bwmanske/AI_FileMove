#include "status_window.hpp"
#include "mainwindow.hpp"
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>

namespace FileMove {

enum ControlId {
    ID_STATUS_OPEN_LOG = 1001,
    ID_STATUS_SELECT_JSON,
    ID_STATUS_NEW_JSON,
    ID_STATUS_PAUSE_RESUME,
    ID_STATUS_LISTVIEW
};

StatusWindow::StatusWindow(HWND hParent, Settings& currentSettings, MoveWorker& worker, MainWindow* pMainWindow)
    : m_hwndParent(hParent), m_settings(currentSettings), m_worker(worker), m_pMainWindow(pMainWindow) {}

StatusWindow::~StatusWindow() {}

void StatusWindow::ShowModal() {
#ifdef _WIN32
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = StatusWindow::WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"StatusWindowClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);

    if (!RegisterClassExW(&wc)) {
        return;
    }

    m_hwndDialog = CreateWindowExW(
        WS_EX_DLGMODALFRAME,
        L"StatusWindowClass",
        L"Status",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 500,
        m_hwndParent, NULL, wc.hInstance, this
    );

    if (m_hwndDialog) {
        CreateControls();
        UpdateUI();

        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
#endif
}

void StatusWindow::CreateControls() {
    // Active Files Section
    CreateWindowExW(0, L"STATIC", L"Active Files:", WS_CHILD | WS_VISIBLE, 15, 15, 150, 20, m_hwndDialog, NULL, GetModuleHandle(NULL), NULL);
    m_hActiveJsonLabel = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE, 15, 40, 350, 20, m_hwndDialog, NULL, GetModuleHandle(NULL), NULL);
    m_hActiveLogLabel = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE, 15, 65, 350, 20, m_hwndDialog, NULL, GetModuleHandle(NULL), NULL);
    CreateWindowExW(0, L"BUTTON", L"Open", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 340, 60, 45, 25, m_hwndDialog, (HMENU)ID_STATUS_OPEN_LOG, GetModuleHandle(NULL), NULL);

    // JSON File List Section
    CreateWindowExW(0, L"STATIC", L"Data Directory Files:", WS_CHILD | WS_VISIBLE, 15, 100, 200, 20, m_hwndDialog, NULL, GetModuleHandle(NULL), NULL);
    m_hJsonListView = CreateWindowExW(0, WC_LISTVIEW, L"", WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | WS_BORDER, 15, 125, 350, 150, m_hwndDialog, (HMENU)ID_STATUS_LISTVIEW, GetModuleHandle(NULL), NULL);
    
    LVCOLUMNW lvc = {};
    lvc.mask = LVCF_TEXT | LVCF_WIDTH;
    lvc.pszText = (LPWSTR)L"JSON File Name";
    lvc.cx = 330;
    ListView_InsertColumn(m_hJsonListView, 0, &lvc);

    CreateWindowExW(0, L"BUTTON", L"Open Selected", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 15, 285, 140, 30, m_hwndDialog, (HMENU)ID_STATUS_SELECT_JSON, GetModuleHandle(NULL), NULL);
    CreateWindowExW(0, L"BUTTON", L"New", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 165, 285, 140, 30, m_hwndDialog, (HMENU)ID_STATUS_NEW_JSON, GetModuleHandle(NULL), NULL);

    // Queue Status Section
    CreateWindowExW(0, L"STATIC", L"Queue Status:", WS_CHILD | WS_VISIBLE, 15, 330, 150, 20, m_hwndDialog, NULL, GetModuleHandle(NULL), NULL);
    m_hQueueStatusLabel = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE, 15, 355, 350, 20, m_hwndDialog, NULL, GetModuleHandle(NULL), NULL);
    m_hWorkerStateLabel = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE, 15, 380, 350, 20, m_hwndDialog, NULL, GetModuleHandle(NULL), NULL);
    m_hPauseResumeButton = CreateWindowExW(0, L"BUTTON", L"Pause / Resume", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 240, 410, 125, 30, m_hwndDialog, (HMENU)ID_STATUS_PAUSE_RESUME, GetModuleHandle(NULL), NULL);
}

void StatusWindow::UpdateUI() {
    // Update active file labels from MainWindow
    std::string jsonPath = m_pMainWindow->GetActiveJsonPath().string();
    std::string logPath = m_pMainWindow->GetActiveLogPath().string();
    SetWindowTextW(m_hActiveJsonLabel, std::wstring(jsonPath.begin(), jsonPath.end()).c_str());
    SetWindowTextW(m_hActiveLogLabel, std::wstring(logPath.begin(), logPath.end()).c_str());

    // Populate JSON list from default directory
    ListView_DeleteAllItems(m_hJsonListView);
    std::filesystem::path dataDir = StorageManager::getDefaultDataDirectory();
    if (std::filesystem::exists(dataDir)) {
        int count = 0;
        for (const auto& entry : std::filesystem::directory_iterator(dataDir)) {
            if (entry.path().extension() == ".json") {
                LVITEMW lvi = {};
                lvi.mask = LVIF_TEXT;
                lvi.iItem = count++;
                std::wstring filename = entry.path().filename().wstring();
                lvi.pszText = (LPWSTR)filename.c_str();
                ListView_InsertItem(m_hJsonListView, &lvi);
            }
        }
    }

    // Update Queue Status
    size_t queued = m_worker.GetQueuedCount();
    size_t processed = m_worker.GetProcessedCount();
    std::string queueInfo = "Queued / Processed: " + std::to_string(queued) + " / " + std::to_string(processed);
    SetWindowTextW(m_hQueueStatusLabel, std::wstring(queueInfo.begin(), queueInfo.end()).c_str());

    std::string workerState = m_worker.GetWorkerState();
    SetWindowTextW(m_hWorkerStateLabel, std::wstring(workerState.begin(), workerState.end()).c_str());
}

void StatusWindow::OnOpenLog() {
    std::string logPath = m_pMainWindow->GetActiveLogPath().string();
    if (!logPath.empty()) {
        ShellExecuteW(NULL, L"open", std::wstring(logPath.begin(), logPath.end()).c_str(), NULL, NULL, SW_SHOW);
    }
}

void StatusWindow::OnSelectJson(const std::string& jsonPath) {
    if (jsonPath.empty()) return;
    std::filesystem::path p(jsonPath);
    if (!std::filesystem::exists(p)) return;

    m_settings = StorageManager::loadSettings(p);
    m_pMainWindow->UpdateStatusLabels(p.string(), (p.parent_path() / (p.stem().string() + ".log")).string());
    PostQuitMessage(0); 
}

void StatusWindow::OnNewJson() {
    std::string name = "NewGroup";
    std::filesystem::path newPath = StorageManager::getDefaultDataDirectory() / (name + ".json");
    // For a real app we would prompt for a name.
    OnSelectJson(newPath.string());
}

void StatusWindow::OnPauseResume() {
    std::string state = m_worker.GetWorkerState();
    if (state == "Manual Pause") {
        m_worker.Resume();
    } else {
        m_worker.Pause();
    }
    UpdateUI();
}

LRESULT CALLBACK StatusWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
#ifdef _WIN32
    StatusWindow* pThis = nullptr;

    if (uMsg == WM_NCCREATE) {
        CREATESTRUCTW* pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
        pThis = reinterpret_cast<StatusWindow*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
        pThis = reinterpret_cast<StatusWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (pThis) {
        switch (uMsg) {
            case WM_COMMAND: {
                int wmId = LOWORD(wParam);
                if (wmId == ID_STATUS_OPEN_LOG) pThis->OnOpenLog();
                else if (wmId == ID_STATUS_SELECT_JSON) {
                    int selectedIdx = ListView_GetNextItem(pThis->m_hJsonListView, -1, LVNI_SELECTED);
                    if (selectedIdx != -1) {
                        wchar_t fileNameBuffer[MAX_PATH] = { 0 };
                        ListView_GetItemTextW(pThis->m_hJsonListView, selectedIdx, 0, fileNameBuffer, MAX_PATH);
                        std::filesystem::path dataDir = StorageManager::getDefaultDataDirectory();
                        std::string fullPath = (dataDir / fileNameBuffer).string();
                        pThis->OnSelectJson(fullPath);
                    }
                } else if (wmId == ID_STATUS_NEW_JSON) pThis->OnNewJson();
                else if (wmId == ID_STATUS_PAUSE_RESUME) pThis->OnPauseResume();
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
