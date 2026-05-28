#ifndef SHUTDOWN_DIALOG_HPP
#define SHUTDOWN_DIALOG_HPP

#include <string>
#include "filemove.hpp"

#ifdef _WIN32
#include <windows.h>
#else
typedef void* HWND;
#endif

namespace FileMove {

enum class ShutdownResult {
    Cancel,
    FinishCurrent,
    Immediate
};

class ShutdownDialog {
public:
    ShutdownDialog(HWND hParent);
    ~ShutdownDialog();

    ShutdownResult ShowModal();

private:
#ifdef _WIN32
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    HWND m_hwndParent;
    HWND m_hwndDialog;
#else
    HWND m_hwndParent;
#endif
};

} // namespace FileMove

#endif // SHUTDOWN_DIALOG_HPP
