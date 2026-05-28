#ifndef QUEUE_WINDOW_HPP
#define QUEUE_WINDOW_HPP

#include <windows.h>
#include "filemove.hpp"

namespace FileMove {

class MainWindow;

class QueueWindow {
public:
    QueueWindow(HWND hParent, MoveWorker& worker, MainWindow* pMainWindow);
    ~QueueWindow();

    void Show(); 

private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    HWND m_hwndParent;
    HWND m_hwndDialog;
    MoveWorker& m_worker;
    MainWindow* m_pMainWindow;

    // UI Elements
    HWND m_hListView = nullptr;
    HWND m_hHeaderLabel = nullptr;

    void CreateControls();
    void UpdateUI();

    // Timer for updates
    static const UINT TIMER_UPDATE_ID = 1000;
};

} // namespace FileMove

#endif // QUEUE_WINDOW_HPP
