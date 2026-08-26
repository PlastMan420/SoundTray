#pragma once

#include "Resource.h"
#include "AudioControl.h"

#define WM_TRAYICON (WM_APP + 1)

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

ATOM RegisterContentWindow(HINSTANCE hInstance);
