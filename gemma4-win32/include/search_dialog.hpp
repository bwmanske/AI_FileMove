#ifndef SEARCH_DIALOG_HPP
#define SEARCH_DIALOG_HPP

#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
// Dummy types for Linux compilation/testing
typedef void* HWND;
typedef void* HINSTANCE;
typedef void* HANDLE;
typedef void* LPVOID;
typedef void* LPSTR;
typedef void* LPWSTR;
typedef void* LPARAM;
typedef void* WPARAM;
typedef unsigned int UINT;
typedef long long LRESULT;
#define CALLBACK 
#endif

namespace FileMove {

class SearchDialog {
public:
    SearchDialog(HWND hParent);
    ~SearchDialog();

    std::string ShowModal();

private:
#ifdef _WIN32
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    HWND m_hwndParent;
    HWND m_hwndDialog;
    HWND m_hEdit;
    std::string m_result;
    bool m_bSuccess = false;
#else
    HWND m_hwndParent;
    std::string m_result;
    bool m_bSuccess = false;
#endif
};

} // namespace FileMove

#endif // SEARCH_DIALOG_HPP
