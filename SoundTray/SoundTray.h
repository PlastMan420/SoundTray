#pragma once

#include "Resource.h"
#include "AudioControl.h"

constexpr int MAX_LOADSTRING = 100;
#define WM_TRAYICON (WM_APP + 1)

// Global Variables:
namespace globals {
    HINSTANCE hInst;                                // current instance
    WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
    WCHAR szWindowClass[MAX_LOADSTRING] = L"soundtray__mainwindow";            // the main window class name
    WCHAR szTrayWindowClass[MAX_LOADSTRING] = L"soundtray__childwindow";      // the tray window class name
    HWND TrayPopup;
    WASAPIAudioManager sAudioDevices = {};
}

// Forward declarations of functions included in this code module:
ATOM                RegisterMainWindow(HINSTANCE hInstance);
ATOM                RegisterTrayWindow(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    PopupWndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

/// <summary>
/// Create tray child window.
/// </summary>
/// <param name="owner"></param>
/// <returns></returns>
HWND CreateTrayPopup(HINSTANCE hInstance, HWND owner);

/// <summary>
/// Show the tray child window about taskbar trigger.
/// </summary>
/// <param name="popup"></param>
void ShowTrayPopup(HWND popup);

/// <summary>
/// Init program tray icon
/// </summary>
/// <param name="hWnd"></param>
/// <param name="hInstance"></param>
/// <returns></returns>
void InitTrayIcon(HWND hWnd, HINSTANCE hInstance);
