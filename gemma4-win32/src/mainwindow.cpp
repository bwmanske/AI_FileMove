#include "mainwindow.hpp"
#include "group_dialog.hpp"
#include "about_window.hpp"
#include "settings_window.hpp"
#include "shutdown_dialog.hpp"
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#include <commctrl.h>
#endif

namespace FileMove {

// Helper to convert wstring to string (needed for groupName extraction)
static std::string ToString(const wchar_t* ws) {
    if (!ws) return "";
    std::wstring w(ws);
    return std::string(w.begin(), w.end());
}

MainWindow::MainWindow(HINSTANCE hInstance, const CommandLineOptions& options)
    : m_hInstance(hInstance), m_hwnd(nullptr), m_options(options) {
#ifdef _WIN32
    if (m_options.debugMode != DebugMode::None) {
        if (AllocConsole()) {
            freopen("CONOUT$", "w", stdout);
            freopen("CONOUT$", "w", stderr);
            FileMove::Logger::SetDebugEnabled(true);
            std::cout << "Debug console enabled." << std::endl;
        }
    }
#endif
}

MainWindow::~MainWindow() {
#ifdef _WIN32
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
    }
#endif
}

bool MainWindow::Create(int nCmdShow) {
#ifdef _WIN32
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = MainWindow::WindowProc;
    wc.hInstance = m_hInstance;
    wc.lpszClassName = L"FileMoveWindowClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if (!RegisterClassExW(&wc)) {
        return false;
    }

    m_hwnd = CreateWindowExW(
        0,
        L"FileMoveWindowClass",
        L"FileMove",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 500,
        NULL, NULL, m_hInstance, this
    );

    if (!m_hwnd) {
        return false;
    }

    ShowWindow(m_hwnd, nCmdShow);
    UpdateWindow(m_hwnd);

    InitializeControls();
    m_worker.Start();

    return true;
#else
    return false;
#endif
}

void MainWindow::RunMessageLoop() {
#ifdef _WIN32
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
#endif
}

#ifdef _WIN32
void MainWindow::InitializeControls() {
    m_hListView = CreateWindowExW(
        0, WC_LISTVIEW, L"",
        WS_CHILD | WS_VISIBLE | LVS_REPORT,
        0, 0, 400, 400, m_hwnd, NULL, m_hInstance, NULL
    );

    DragAcceptFiles(m_hListView, TRUE);

    // Initialize Tooltip Control
    m_hTooltip = CreateWindowExW(0, TOOLTIP_CLASSW, L"",
        WS_CHILD | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        m_hwnd, NULL, m_hInstance, NULL);

    TOOLINFO ti = {0};
    ti.cbSize = sizeof(TOOLINFO);
    ti.hwnd = m_hwnd;
    ti.uId = (UINT)m_hListView;
    ti.uFlags = TTF_SUBCLASS | TTF_TRACK;
    ti.pszWindowText = (LPWSTR)L"";
    SendMessage(m_hTooltip, TTM_ADDTOOL, 0, (LPARAM)&ti);

    m_hStatusLabelLeft = CreateWindowW(L"STATIC", L"Queued: 0", WS_CHILD | WS_VISIBLE,
        10, 470, 150, 20, m_hwnd, NULL, m_hInstance, NULL);

    m_hStatusLabelRight = CreateWindowW(L"STATIC", L"Status: Idle", WS_CHILD | WS_VISIBLE,
        230, 470, 150, 20, m_hwnd, NULL, m_hInstance, NULL);
}
#endif

void MainWindow::UpdateStatusLabels(const std::string& jsonPath, const std::string& logPath) {
#ifdef _WIN32
    if (!jsonPath.empty()) {
        m_activeJsonPath = jsonPath;
        m_activeLogPath = logPath;

        // Log JSON file open time (session record)
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
        FileMove::Logger::LogSession(logPath, "JSON file opened: " + jsonPath);

        FileMove::Logger::LogDebug("JSON file opened: " + jsonPath);
        FileMove::Logger::LogDebug("Active log file changed to: " + logPath);
    }

    if (m_hStatusLabelLeft) {
        std::string queuedStr = "Queued: " + std::to_string(m_worker.GetQueuedCount());
        SetWindowTextW(m_hStatusLabelLeft, std::wstring(queuedStr.begin(), queuedStr.end()).c_str());
    }
    if (m_hStatusLabelRight) {
        std::string statusStr = "Status: " + m_worker.GetWorkerState();
        SetWindowTextW(m_hStatusLabelRight, std::wstring(statusStr.begin(), statusStr.end()).c_str());
    }
#endif
}

#ifdef _WIN32
LRESULT CALLBACK MainWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    MainWindow* pThis = nullptr;

    if (uMsg == WM_NCCREATE) {
        CREATESTRUCTW* pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
        pThis = reinterpret_cast<MainWindow*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
        pThis = reinterpret_cast<MainWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (pThis) {
        switch (uMsg) {
            case WM_COMMAND: {
                int wmId = LOWORD(wParam);
                if (wmId == 10) { // Use Clipboard
                    if (pThis->m_hListView != nullptr) {
                        int selectedIdx = ListView_GetNextItem(pThis->m_hListView, -1, LVNI_SELECTED);
                        if (selectedIdx != -1) {
                            wchar_t groupBuffer[256];
                            ListView_GetItemTextW(pThis->m_hListView, selectedIdx, 0, groupBuffer, 256);
                            std::string groupName = ToString(groupBuffer);

                            const Group* selectedGroup = nullptr;
                            for (const auto& g : pThis->m_settings.groups) {
                                if (g.name == groupName) {
                                    selectedGroup = &g;
                                    break;
                                }
                            }

                            if (selectedGroup && !selectedGroup->destinationPaths.empty()) {
                                if (OpenClipboard(pThis->m_hwnd)) {
                                    if (IsClipboardFormatAvailable(CF_HDROP)) {
                                        HDROP hDrop = (HDROP)GetClipboardData(CF_HDROP);
                                        if (hDrop != NULL) {
                                            UINT fileCount = DragQueryFileW(hDrop, 0xFFFFFFFF, NULL, 0);
                                            std::vector<PendingMoveEntry> batch;

                                            auto now = std::chrono::system_clock::now();
                                            auto in_time_t = std::chrono::system_clock::to_time_t(now);
                                            std::stringstream ss;
                                            ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
                                            std::string timestamp = ss.str();

                                            for (UINT i = 0; i < fileCount; ++i) {
                                                wchar_t filePathBuffer[MAX_PATH];
                                                DragQueryFileW(hDrop, i, filePathBuffer, MAX_PATH);
                                                std::filesystem::path filePath(filePathBuffer);

                                                PendingMoveEntry entry;
                                                entry.id = "clip-" + std::to_string(i);
                                                entry.groupId = selectedGroup->id;
                                                entry.sourceFilePath = filePath;
                                                 entry.relativePath = filePath.filename();
                                                entry.destinationDirectories = selectedGroup->destinationPaths;
                                                entry.debugTransferMode = pThis->m_options.debugMode;
                                                entry.queuedAt = timestamp;
                                                entry.status = MoveStatus::Queued;

                                                batch.push_back(entry);
                                            }

                                            if (!batch.empty()) {
                                                auto preparedBatch = pThis->m_worker.PrepareBatch(batch);
                                                if (preparedBatch) {
                                                    pThis->m_worker.QueueBatch(*preparedBatch);
                                                } else {
                                                    MessageBoxW(pThis->m_hwnd, L"Validation failed for batch.", L"Error", MB_OK | MB_ICONERROR);
                                                }
                                            }
                                        }
                                        DragFinish(hDrop);
                                    }
                                    CloseClipboard();
                                }
                            } else {
                                MessageBoxW(pThis->m_hwnd, L"Invalid group or no destinations.", L"Error", MB_OK | MB_ICONERROR);
                            }
                        }
                    }
                } else if (wmId == 4) { // New
                    {
                        GroupEditData initialData = {"", {}};
                        GroupEditData resultData;
                        GroupDialog dialog(pThis->m_hwnd, resultData);
                        if (dialog.ShowModal(initialData)) {
                            // Implementation for adding new group should be here.
                        }
                    }
                    break;
                } else if (wmId == 6) { // Queue Window
                    pThis->m_queueWindow.Show();
                    break;
                } else if (wmId == 5) { // About
                    AboutWindow about(pThis->m_hwnd, pThis->m_cmdLine);
                    about.ShowModal();
                    break;
                } else if (wmId == 1) { // Settings
                    SettingsWindow settings(pThis->m_hwnd, pThis->m_settings);
                    settings.ShowModal();
                    FileMove::Logger::LogDebug("Settings window opened.");
                } else if (wmId == 2) { // Status
                    {
                        #include "status_window.hpp"
                        StatusWindow status(pThis->m_hwnd, pThis->m_settings, pThis->m_worker, pThis);
                        status.ShowModal();
                        FileMove::Logger::LogDebug("Status window opened.");
                    }
                } else if (wmId == 3) { // Search
                    // Placeholder for search window
                    FileMove::Logger::LogDebug("Search dialog requested.");
                }
                return 0;
            }
            case WM_DROPFILES: {
                HDROP hDrop = reinterpret_cast<HDROP>(lParam);
                HWND hListView = pThis->m_hListView;
                if (hListView != nullptr) {
                    POINT pt;
                    GetCursorPos(&pt);
                    ScreenToClient(pThis->m_hwnd, &pt);

                    int itemIdx = ListView_HitTest(hListView, pt.x, pt.y);
                    if (itemIdx != -1) {
                        wchar_t groupBuffer[256];
                        ListView_GetItemTextW(hListView, itemIdx, 0, groupBuffer, 256);
                        std::string groupName = ToString(groupBuffer);

                        const Group* selectedGroup = nullptr;
                        for (const auto& g : pThis->m_settings.groups) {
                            if (g.name == groupName) {
                                selectedGroup = &g;
                                break;
                            }
                        }

                        if (selectedGroup && !selectedGroup->destinationPaths.empty()) {
                            UINT fileCount = DragQueryFileW(hDrop, 0xFFFFFFFF, NULL, 0);
                            std::vector<PendingMoveEntry> batch;

                            auto now = std::chrono::system_clock::now();
                            auto in_time_t = std::chrono::system_clock::to_time_t(now);
                            std::stringstream ss;
                            ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
                            std::string timestamp = ss.str();

                            for (UINT i = 0; i < fileCount; ++i) {
                                wchar_t filePathBuffer[MAX_PATH];
                                DragQueryFileW(hDrop, i, filePathBuffer, MAX_PATH);
                                std::filesystem::path filePath(filePathBuffer);

                                PendingMoveEntry entry;
                                entry.id = "drop-" + std::to_string(i);
                                entry.groupId = selectedGroup->id;
                                entry.sourceFilePath = filePath;
                                                 entry.relativePath = filePath.filename();
                                entry.destinationDirectories = selectedGroup->destinationPaths;
                                entry.debugTransferMode = pThis->m_options.debugMode;
                                entry.queuedAt = timestamp;
                                entry.status = MoveStatus::Queued;

                                batch.push_back(entry);
                            }

                            if (!batch.empty()) {
                                auto preparedBatch = pThis->m_worker.PrepareBatch(batch);
                                if (preparedBatch) {
                                    pThis->m_worker.QueueBatch(*preparedBatch);
                                } else {
                                    MessageBoxW(pThis->m_hwnd, L"Validation failed for batch.", L"Error", MB_OK | MB_ICONERROR);
                                }
                            }
                        }
                    }
                }
                DragFinish(hDrop);
                return 0;
            }
            case WM_NOTIFY: {
                LPNMHDR lpnmh = reinterpret_cast<LPNMHDR>(lParam);
                if (lpnmh->hwndFrom == pThis->m_hListView) {
                    if (lpnmh->code == NM_RCLICK) {
                        LPNMLISTVIEW lpnmv = reinterpret_cast<LPNMLISTVIEW>(lParam);
                        int itemIdx = lpnmv->iItem;
                        if (itemIdx != -1) {
                            if (itemIdx == 0) {
                                HMENU hMenu = CreatePopupMenu();
                                AppendMenuW(hMenu, MF_STRING, 1, L"Settings");
                                AppendMenuW(hMenu, MF_STRING, 2, L"Status");
                                AppendMenuW(hMenu, MF_STRING, 3, L"Search");
                                AppendMenuW(hMenu, MF_STRING, 4, L"New");
                                AppendMenuW(hMenu, MF_STRING, 5, L"About");
                                wchar_t queueText[64];
                                swprintf(queueText, 64, L"Queue Window (%zu)", pThis->m_worker.GetQueuedCount());
                                AppendMenuW(hMenu, MF_STRING, 6, queueText);

                                POINT pt;
                                GetCursorPos(&pt);
                                TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, pThis->m_hwnd, NULL);
                                DestroyMenu(hMenu);
                            } else {
                                HMENU hMenu = CreatePopupMenu();
                                AppendMenuW(hMenu, MF_STRING, 10, L"Use Clipboard");
                                AppendMenuW(hMenu, MF_STRING, 11, L"Edit");
                                AppendMenuW(hMenu, MF_STRING, 12, L"Delete");

                                POINT pt;
                                GetCursorPos(&pt);
                                TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, pThis->m_hwnd, NULL);
                                DestroyMenu(hMenu);
                            }
                        }
                    } else if (lpnmh->code == TTN_NEEDTEXTW) {
                        LPNMTTNEEDTEXTW lpntt = reinterpret_cast<LPNMTTNEEDTEXTW>(lParam);
                        int itemIdx = ListView_GetNextItem(pThis->m_hListView, -1, LVNI_SELECTED); // This is wrong for tooltip. Need to use the item under mouse.
                        // Actually, TTN_NEEDTEXT is sent when a tooltip is about to be displayed. 
                        // For a ListView, we need to find which item is at the cursor position.
                        
                        POINT pt;
                        GetCursorPos(&pt);
                        ScreenToClient(pThis->m_hListView, &pt);
                        int itemIdxAtMouse = ListView_HitTest(pThis->m_hListView, pt.x, pt.y);
                        
                        if (itemIdxAtMouse != -1) {
                            wchar_t groupBuffer[256];
                            ListView_GetItemTextW(pThis->m_hListView, itemIdxAtMouse, 0, groupBuffer, 256);
                            std::wstring groupNameW(groupBuffer);

                            const Group* selectedGroup = nullptr;
                            for (const auto& g : pThis->m_settings.groups) {
                                if (g.name == ToString(groupBuffer)) {
                                    selectedGroup = &g;
                                    break;
                                }
                            }

                            if (selectedGroup) {
                                std::wstring tooltipText = groupNameW + L"\n";
                                for (const auto& dest : selectedGroup->destinationPaths) {
                                    std::wstring statusIcon;
                                    try {
                                        if (std::filesystem::exists(dest)) {
                                            statusIcon = L"✓";
                                        } else {
                                            statusIcon = L"X";
                                        }
                                    } catch (...) {
                                        statusIcon = L"?";
                                    }
                                    tooltipText += statusIcon + L" " + dest.wstring() + L"\n";
                                }
                                // Remove last newline if needed or just let it be.
                                wcsncpy(lpntt->pszText, tooltipText.c_str(), lpntt->cchTextMax);
                            }
                        }
                    }
                }
                return 0;
            }
            case WM_CLOSE: {
                if (pThis->m_worker.GetQueuedCount() > 0 || pThis->m_worker.GetWorkerState() != "Idle") {
                    ShutdownDialog dialog(pThis->m_hwnd);
                    ShutdownResult result = dialog.ShowModal();

                    if (result == ShutdownResult::FinishCurrent) {
                        pThis->m_worker.RequestShutdown(ShutdownMode::FinishCurrent);
                    } else if (result == ShutdownResult::Immediate) {
                        pThis->m_worker.RequestShutdown(ShutdownMode::Immediate);
                    } else {
                        // Cancel shutdown
                        return 0;
                    }
                }
                
                pThis->m_worker.Stop();
                DestroyWindow(hwnd);
                return 0;
            }
            case WM_DESTROY:
                pThis->m_worker.Stop();
                PostQuitMessage(0);
                return 0;

        }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
#endif
}

    if (pThis) {
        switch (uMsg) {
            case WM_COMMAND: {
                int wmId = LOWORD(wParam);
                if (wmId == 10) { // Use Clipboard
                    if (pThis->m_hListView != nullptr) {
                        int selectedIdx = ListView_GetNextItem(pThis->m_hListView, -1, LVNI_SELECTED);
                        if (selectedIdx != -1) {
                            wchar_t groupBuffer[256];
                            ListView_GetItemTextW(pThis->m_hListView, selectedIdx, 0, groupBuffer, 256);
                            std::string groupName = ToString(groupBuffer);

                            const Group* selectedGroup = nullptr;
                            for (const auto& g : pThis->m_settings.groups) {
                                if (g.name == groupName) {
                                    selectedGroup = &g;
                                    break;
                                }
                            }

                            if (selectedGroup && !selectedGroup->destinationPaths.empty()) {
                                if (OpenClipboard(pThis->m_hwnd)) {
                                    if (IsClipboardFormatAvailable(CF_HDROP)) {
                                        HDROP hDrop = (HDROP)GetClipboardData(CF_HDROP);
                                        if (hDrop != NULL) {
                                            UINT fileCount = DragQueryFileW(hDrop, 0xFFFFFFFF, NULL, 0);
                                            std::vector<PendingMoveEntry> batch;

                                            auto now = std::chrono::system_clock::now();
                                            auto in_time_t = std::chrono::system_clock::to_time_t(now);
                                            std::stringstream ss;
                                            ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
                                            std::string timestamp = ss.str();

                                            for (UINT i = 0; i < fileCount; ++i) {
                                                wchar_t filePathBuffer[MAX_PATH];
                                                DragQueryFileW(hDrop, i, filePathBuffer, MAX_PATH);
                                                std::filesystem::path filePath(filePathBuffer);

                                                PendingMoveEntry entry;
                                                entry.id = "clip-" + std::to_string(i);
                                                entry.groupId = selectedGroup->id;
                                                entry.sourceFilePath = filePath;
                                                 entry.relativePath = filePath.filename();
                                                entry.destinationDirectories = selectedGroup->destinationPaths;
                                                entry.debugTransferMode = pThis->m_options.debugMode;
                                                entry.queuedAt = timestamp;
                                                entry.status = MoveStatus::Queued;

                                                batch.push_back(entry);
                                            }

                                            if (!batch.empty()) {
                                                auto preparedBatch = pThis->m_worker.PrepareBatch(batch);
                                                if (preparedBatch) {
                                                    pThis->m_worker.QueueBatch(*preparedBatch);
                                                } else {
                                                    MessageBoxW(pThis->m_hwnd, L"Validation failed for batch.", L"Error", MB_OK | MB_ICONERROR);
                                                }
                                            }
                                        }
                                        DragFinish(hDrop);
                                    }
                                    CloseClipboard();
                                }
                            } else {
                                MessageBoxW(pThis->m_hwnd, L"Invalid group or no destinations.", L"Error", MB_OK | MB_ICONERROR);
                            }
                        }
                    }
                 } else if (wmId == 4) { // New
                     {
                         GroupEditData initialData = {"", {}};
                         GroupEditData resultData;
                         GroupDialog dialog(pThis->m_hwnd, resultData);
                         if (dialog.ShowModal(initialData)) {
                             // Implementation for adding new group should be here.
                         }
                     }
                     break;
                 } else if (wmId == 6) { // Queue Window
                     pThis->m_queueWindow.Show();
                     break;
                 } else if (wmId == 5) { // About
                     AboutWindow about(pThis->m_hwnd, pThis->m_cmdLine);
                     about.ShowModal();
                     break;
                 } else if (wmId == 1) { // Settings
                     SettingsWindow settings(pThis->m_hwnd, pThis->m_settings);
                     settings.ShowModal();
                     FileMove::Logger::LogDebug("Settings window opened.");
                 } else if (wmId == 2) { // Status
                     {
                         #include "status_window.hpp"
                         StatusWindow status(pThis->m_hwnd, pThis->m_settings, pThis->m_worker, pThis);
                         status.ShowModal();
                         FileMove::Logger::LogDebug("Status window opened.");
                     }
                 } else if (wmId == 3) { // Search
                     // Placeholder for search window
                     FileMove::Logger::LogDebug("Search dialog requested.");
                 }
                return 0;
            }
            case WM_DROPFILES: {
                HDROP hDrop = reinterpret_cast<HDROP>(lParam);
                HWND hListView = pThis->m_hListView;
                if (hListView != nullptr) {
                    POINT pt;
                    GetCursorPos(&pt);
                    ScreenToClient(pThis->m_hwnd, &pt);

                    int itemIdx = ListView_HitTest(hListView, pt.x, pt.y);
                    if (itemIdx != -1) {
                        wchar_t groupBuffer[256];
                        ListView_GetItemTextW(hListView, itemIdx, 0, groupBuffer, 256);
                        std::string groupName = ToString(groupBuffer);

                        const Group* selectedGroup = nullptr;
                        for (const auto& g : pThis->m_settings.groups) {
                            if (g.name == groupName) {
                                selectedGroup = &g;
                                break;
                            }
                        }

                        if (selectedGroup && !selectedGroup->destinationPaths.empty()) {
                            UINT fileCount = DragQueryFileW(hDrop, 0xFFFFFFFF, NULL, 0);
                            std::vector<PendingMoveEntry> batch;

                            auto now = std::chrono::system_clock::now();
                            auto in_time_t = std::chrono::system_clock::to_time_t(now);
                            std::stringstream ss;
                            ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
                            std::string timestamp = ss.str();

                            for (UINT i = 0; i < fileCount; ++i) {
                                wchar_t filePathBuffer[MAX_PATH];
                                DragQueryFileW(hDrop, i, filePathBuffer, MAX_PATH);
                                std::filesystem::path filePath(filePathBuffer);

                                PendingMoveEntry entry;
                                entry.id = "drop-" + std::to_string(i);
                                entry.groupId = selectedGroup->id;
                                entry.sourceFilePath = filePath;
                                                 entry.relativePath = filePath.filename();
                                entry.destinationDirectories = selectedGroup->destinationPaths;
                                entry.debugTransferMode = pThis->m_options.debugMode;
                                entry.queuedAt = timestamp;
                                entry.status = MoveStatus::Queued;

                                batch.push_back(entry);
                            }

                            if (!batch.empty()) {
                                auto preparedBatch = pThis->m_worker.PrepareBatch(batch);
                                if (preparedBatch) {
                                    pThis->m_worker.QueueBatch(*preparedBatch);
                                } else {
                                    MessageBoxW(pThis->m_hwnd, L"Validation failed for batch.", L"Error", MB_OK | MB_ICONERROR);
                                }
                            }
                        }
                    }
                }
                DragFinish(hDrop);
                return 0;
            }
            case WM_NOTIFY: {
                LPNMHDR lpnmh = reinterpret_cast<LPNMHDR>(lParam);
                if (lpnmh->hwndFrom == pThis->m_hListView) {
                    if (lpnmh->code == NM_RCLICK) {
                        LPNMLISTVIEW lpnmv = reinterpret_cast<LPNMLISTVIEW>(lParam);
                        int itemIdx = lpnmv->iItem;
                        if (itemIdx != -1) {
                            if (itemIdx == 0) {
                                HMENU hMenu = CreatePopupMenu();
                                 AppendMenuW(hMenu, MF_STRING, 1, L"Settings");
                                 AppendMenuW(hMenu, MF_STRING, 2, L"Status");
                                 AppendMenuW(hMenu, MF_STRING, 3, L"Search");
                                 AppendMenuW(hMenu, MF_STRING, 4, L"New");
                                 AppendMenuW(hMenu, MF_STRING, 5, L"About");
                                 wchar_t queueText[64];
                                 swprintf(queueText, 64, L"Queue Window (%zu)", pThis->m_worker.GetQueuedCount());
                                 AppendMenuW(hMenu, MF_STRING, 6, queueText);

                                POINT pt;
                                GetCursorPos(&pt);
                                TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, pThis->m_hwnd, NULL);
                                DestroyMenu(hMenu);
                            } else {
                                HMENU hMenu = CreatePopupMenu();
                                AppendMenuW(hMenu, MF_STRING, 10, L"Use Clipboard");
                                AppendMenuW(hMenu, MF_STRING, 11, L"Edit");
                                AppendMenuW(hMenu, MF_STRING, 12, L"Delete");

                                POINT pt;
                                GetCursorPos(&pt);
                                TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, pThis->m_hwnd, NULL);
                                DestroyMenu(hMenu);
                            }
                        }
                    } else if (lpnmh->code == TTN_NEEDTEXTW) {
                        LPNMTTNEEDTEXTW lpntt = reinterpret_cast<LPNMTTNEEDTEXTW>(lParam);
                        int itemIdx = ListView_GetNextItem(pThis->m_hListView, -1, LVNI_SELECTED); // This is wrong for tooltip. Need to use the item under mouse.
                        // Actually, TTN_NEEDTEXT is sent when a tooltip is about to be displayed. 
                        // For a ListView, we need to find which item is at the cursor position.
                        
                        POINT pt;
                        GetCursorPos(&pt);
                        ScreenToClient(pThis->m_hListView, &pt);
                        int itemIdxAtMouse = ListView_HitTest(pThis->m_hListView, pt.x, pt.y);

                        if (itemIdxAtMouse != -1) {
                            wchar_t groupBuffer[256];
                            ListView_GetItemTextW(pThis->m_hListView, itemIdxAtMouse, 0, groupBuffer, 256);
                            std::wstring groupNameW(groupBuffer);

                            const Group* selectedGroup = nullptr;
                            for (const auto& g : pThis->m_settings.groups) {
                                if (g.name == ToString(groupBuffer)) {
                                    selectedGroup = &g;
                                    break;
                                }
                            }

                            if (selectedGroup) {
                                 std::wstring tooltipText = groupNameW + L"\n";
                                 for (const auto& dest : selectedGroup->destinationPaths) {
                                     std::wstring statusIcon;
                                     try {
                                         if (std::filesystem::exists(dest)) {
                                             statusIcon = L"✓";
                                         } else {
                                             statusIcon = L"X";
                                         }
                                     } catch (...) {
                                         statusIcon = L"?";
                                     }
                                     tooltipText += statusIcon + L" " + dest.wstring() + L"\n";
                                 }
                                 // Remove last newline if needed or just let it be.
                                 wcsncpy(lpntt->pszText, tooltipText.c_str(), lpntt->cchTextMax);
                            }
                        }
                    }
                }
                return 0;
            }
                        }
                    }
                }
                return 0;
            }
            case WM_CLOSE: {
                if (pThis->m_worker.GetQueuedCount() > 0 || pThis->m_worker.GetWorkerState() != "Idle") {
                    ShutdownDialog dialog(pThis->m_hwnd);
                    ShutdownResult result = dialog.ShowModal();

                    if (result == ShutdownResult::FinishCurrent) {
                        pThis->m_worker.RequestShutdown(ShutdownMode::FinishCurrent);
                    } else if (result == ShutdownResult::Immediate) {
                        pThis->m_worker.RequestShutdown(ShutdownMode::Immediate);
                    } else {
                        // Cancel shutdown
                        return 0;
                    }
                }
                
                pThis->m_worker.Stop();
                DestroyWindow(hwnd);
                return 0;
            }
            case WM_DESTROY:
                pThis->m_worker.Stop();
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
