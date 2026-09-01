#include "pch.h"
#include "global.h"
#include "winrt/impl/Microsoft.UI.Xaml.Hosting.2.h"

namespace globals {
    HINSTANCE hInst = NULL;

    WCHAR szTitle[MAX_LOADSTRING] = L"SoundTray";
    WCHAR szWindowClass[MAX_LOADSTRING] = L"soundtray__mainwindow";
    WCHAR szTrayWindowClass[MAX_LOADSTRING] = L"soundtray__childwindow";
    WCHAR szHotKeyWindowClass[MAX_LOADSTRING] = L"soundtray__HotkeyWindow";
    
    HWND hMainWindow = NULL;
    HWND hTrayContent = NULL;

    NOTIFYICONDATA g_nid = {};
    UINT iTrayHotKey = VK_UP;

    winrt::Microsoft::UI::Xaml::Hosting::DesktopWindowXamlSource popupXamlSource{ nullptr };
}
