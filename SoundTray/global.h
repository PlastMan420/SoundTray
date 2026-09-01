#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "winrt/impl/Microsoft.UI.Xaml.Hosting.2.h"

constexpr int MAX_LOADSTRING = 100;

// Global Variables:
namespace globals {
    extern HINSTANCE hInst;                                // current instance

    extern WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
    extern WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
    extern WCHAR szTrayWindowClass[MAX_LOADSTRING];      // the tray window class name
    extern WCHAR szHotKeyWindowClass[MAX_LOADSTRING];

    extern HWND hTrayContent;
    extern HWND hMainWindow;

    extern NOTIFYICONDATA g_nid;
    extern UINT iTrayHotKey;

    // The WinUI 3/XAML object that hosts your MainPage inside the existing Win32 popup.
    extern winrt::Microsoft::UI::Xaml::Hosting::DesktopWindowXamlSource
        popupXamlSource{ nullptr };
}
