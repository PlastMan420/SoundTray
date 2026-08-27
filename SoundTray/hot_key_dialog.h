#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

/// <summary>
/// Assign new ID_HOTKEY_EXPAND hotkey to a window. CTRL+SHIFT combo is always there
/// </summary>
/// <param name="hTrayWindow"></param>
/// <param name="iKeyCombo"></param>
/// <returns></returns>
BOOL RegisterPopUpTrayHotKey(HWND hTrayWindow, UINT iKeyCombo);

/// <summary>
/// Unassign ID_HOTKEY_EXPAND hotkey from a window
/// </summary>
/// <param name="hTrayWindow"></param>
void UnRegisterPopUpTrayHotKey(HWND hTrayWindow);

/// <summary>
/// On enter input in register new hotkey dialog
/// </summary>
/// <param name="hDlg"></param>
/// <param name="uMsg"></param>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <returns></returns>
LRESULT CALLBACK HotkeyWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void CreateHotkeyWindow(HINSTANCE hInstance);

ATOM RegisterHotkeyWindowClass(HINSTANCE hInstance);

BOOL UpdatePopUpTrayHotKey(HWND hTrayWindow, UINT iKeyCombo);
