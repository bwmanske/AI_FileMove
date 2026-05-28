#ifndef GROUP_DIALOG_HPP
#define GROUP_DIALOG_HPP

#include <string>
#include <vector>
#include <filesystem>
#include "filemove.hpp"

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

struct GroupEditData {
    std::string name;
    std::vector<std::filesystem::path> destinations;
};

class GroupDialog {
public:
    GroupDialog(HWND hParent, GroupEditData& resultData);
    ~GroupDialog();

    bool ShowModal(const GroupEditData& initialData);

private:
#ifdef _WIN32
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    HWND m_hwndParent;
    HWND m_hwndDialog;

    // UI Elements
    HWND m_hNameEdit;
    HWND m_hDestListView;
    HWND m_hStatusLabel; // To show error messages if any
    HIMAGELIST m_hImageList = nullptr;
    ULONG_PTR m_gdiToken = 0;
    
    bool m_bSaved = false;
    GroupEditData& m_resultData; 
#else
    HWND m_hwndParent;
    GroupEditData& m_resultData;
    bool m_bSaved = false;
#endif

    void CreateControls();
    void ValidateDestinations();
};

} // namespace FileMove

#endif // GROUP_DIALOG_HPP
