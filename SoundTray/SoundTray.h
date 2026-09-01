#pragma once

// Forward declarations of functions included in this code module:
ATOM                RegisterMainWindow(HINSTANCE hInstance);

HWND                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    PopupWndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

/// <summary>
/// Create tray child window.
/// </summary>
/// <param name="owner"></param>
/// <returns></returns>
HWND CreatehTrayPopup(HINSTANCE hInstance, HWND owner);

/// <summary>
/// Init program tray icon
/// </summary>
/// <param name="hWnd"></param>
/// <param name="hInstance"></param>
/// <returns></returns>
void InitTrayIcon(HWND hWnd, HINSTANCE hInstance);

UINT LoadHotkey();

/// <summary>
/// <para>This code solves a specific problem caused by mixing Win32 controls and a WinUI 3 XAML Island in the same window: 
/// making Tab and Shift+Tab keyboard navigation cross the boundary between Win32 and XAML correctly.</para>
/// <para>
/// Win32's keyboard focus system knows about the Win32 HWNDs.
/// 
/// WinUI's XAML focus system knows about the XAML elements.
/// 
/// They aren't the same focus system.
/// 
/// So if the user presses :
/// 
/// Tab
/// 
/// Win32 might determine :
/// 
/// Win32 Edit → XAML Island
/// 
/// but simply doing :
/// 
/// SetFocus(xamlIslandHwnd);
/// 
/// doesn't tell XAML which XAML element should receive focus.
/// 
/// That's what NavigateFocus() fixes.
/// </para>
/// </summary>
/// <param name="topLevelWindow"></param>
/// <param name="msg"></param>
/// <returns></returns>
bool ProcessMessageForTabNavigation(const HWND topLevelWindow, MSG* msg);
