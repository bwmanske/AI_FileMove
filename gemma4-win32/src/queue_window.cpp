#include "queue_window.hpp"
#include "mainwindow.hpp"
#include <string>
#include <vector>
#include <filesystem>

namespace FileMove {

QueueWindow::QueueWindow(HWND hParent, MoveWorker& worker, MainWindow* pMainWindow)
    : m_hwndParent(hParent), m_worker(worker), m_pMainWindow(pMainWindow), m_hwndDialog(nullptr) {}

QueueWindow::~QueueWindow() {}

void QueueWindow::Show() {
#ifdef _WIN32
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = QueueWindow::WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"QueueWindowClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);

    if (!RegisterClassExW(&wc)) {
        return;
    }

    m_hwndDialog = CreateWindowExW(
        WS_EX_DLGMODALFRAME,
        L"QueueWindowClass",
        L"Queue",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 300,
        m_hwndParent, NULL, wc.hInstance, this
    );

    if (m_hwndDialog) {
        CreateControls();
        UpdateUI();
        SetTimer(m_hwndDialog, TIMER_UPDATE_ID, 500, NULL);
    }
#endif
}

void QueueWindow::CreateControls() {
#ifdef _WIN32
    HWND hHeader = CreateWindowExW(0, L"STATIC", L"Queued Destination Paths:", WS_CHILD | WS_VISIBLE, 15, 10, 350, 20, m_hwndDialog, NULL, GetModuleHandle(NULL), NULL);
    m_hHeaderLabel = hHeader;

    HWND m_hListView = CreateWindowExW(0, WC_LISTVIEW, L"", WS_CHILD | WS_VISIBLE | LVS_REPORT | WS_BORDER, 15, 40, 370, 220, m_hwndDialog, NULL, GetModuleHandle(NULL), NULL);

    LVCOLUMNW lvc = {};
    lvc.mask = LVCF_TEXT | LVCF_WIDTH;
    lvc.pszText = (LPWSTR)L"Source File";
    lvc.cx = 150;
    ListView_InsertColumn(m_hListView, 0, &lvc);

    lvc.pszText = (LPWSTR)L"Destinations";
    lvc.cx = 200;
    ListView_InsertColumn(m_hListView, 1, &lvc);
#endif
}

void QueueWindow::UpdateUI() {
#ifdef _WIN32
    size_t queued = m_worker.GetQueuedCount();
    std::string queuedStr = std::to_string(queued);
    std::wstring headerText = L"Queued Items (" + std::wstring(queuedStr.begin(), queuedStr.end()) + L")";
    SetWindowTextW(m_hHeaderLabel, headerText.c_str());

    ListView_DeleteAllItems(m_hListView);

    auto items = m_worker.GetQueuedItems();
    int index = 0;
    for (const auto& item : items) {
        LVITEMW lvi = {0};
        lvi.mask = LVIF_TEXT;
        lvi.iItem = index;

        std::wstring sourceName = item.sourceFilePath.filename().wstring();
        ListView_InsertItemW(m_hListView, index, sourceName.c_str());

        std::wstring dests = L"";
        for (const auto& d : item.destinationDirectories) {
            if (!dests.empty()) dests += L", ";
            dests += d.filename().wstring(); // Just showing filename for brevity in queue window, maybe full path is better? 
                                            // Actually, the spec says "real-time queue monitoring". 
                                            // Let's use the parent path + filename or just filename if it's clear.
        }
        ListView_SetItemTextW(m_hListView, index, 1, dests.c_str());

        index++;
    }
#endif
}

        ListView_InsertItemW(m_hListView, index, displayText.c_str());
        index++;
    }
#endif
}

LRESULT CALLBACK QueueWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
#ifdef _WIN32
    QueueWindow* pThis = nullptr;

    if (uMsg == WM_NCCREATE) {
        CREATESTRUCTW* pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
        pThis = reinterpret_cast<QueueWindow*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
        pThis = reinterpret_cast<QueueWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (pThis) {
        switch (uMsg) {
            case WM_TIMER:
                if (wParam == TIMER_UPDATE_ID) {
                    pThis->UpdateUI();
                }
                return 0;

            case WM_DESTROY:
                KillTimer(hwnd, TIMER_UPDATE_ID);
                DestroyWindow(hwnd);
                return 0;
        }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
#else
    return 0;
#endif
}

} // namespace FileMove
