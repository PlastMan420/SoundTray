#include "pch.h"
#include "hot_key_dialog.h"
#include "global.h"
#include "Resource.h"
#include "config.h"

ATOM RegisterHotkeyWindowClass(HINSTANCE hInstance) {
    WNDCLASSW wc{};
    wc.lpfnWndProc = HotkeyWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = globals::szHotKeyWindowClass;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    return RegisterClassW(&wc);
}

void CreateHotkeyWindow(HINSTANCE hInstance) {
    HWND hHotkeyWnd = CreateWindowExW(
        WS_EX_TOPMOST,
        globals::szHotKeyWindowClass,
        L"Set Hotkey",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        300,
        120,
        nullptr,
        nullptr,
        hInstance,
        globals::hMainWindow
    );

    HWND hHotkeyText = CreateWindowExW(
        0,
        L"STATIC",
        L"CTRL + SHIFT +",
        WS_CHILD | WS_VISIBLE,
        10, 10, 100, 12,
        hHotkeyWnd,
        nullptr,
        hInstance,
        nullptr
    );

    HWND hHotkeyKey = CreateWindowExW(
        0,
        L"STATIC",
        L"Press a key...",
        WS_CHILD | WS_VISIBLE,
        10, 30, 150, 12,
        hHotkeyWnd,
        nullptr,
        hInstance,
        nullptr
    );

    ShowWindow(hHotkeyWnd, SW_SHOW);
    SetForegroundWindow(hHotkeyWnd);
    SetFocus(hHotkeyWnd);
}

LRESULT CALLBACK HotkeyWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_CREATE:
            SetFocus(hWnd);
            return 0;

        case WM_KEYDOWN:
        {
            UINT vk = static_cast<UINT>(wParam);

            // Ignore modifier keys.
            if (vk == VK_CONTROL ||
                vk == VK_SHIFT ||
                vk == VK_MENU)
            {
                return 0;
            }

            UnRegisterPopUpTrayHotKey(globals::hMainWindow);

            if (UpdatePopUpTrayHotKey(globals::hMainWindow, vk))
            {
                DestroyWindow(hWnd);
            }

            return 0;
        }

        case WM_CLOSE:
            DestroyWindow(hWnd);
            return 0;

        case WM_DESTROY:
            return 0;
    }

    return DefWindowProcW(hWnd, message, wParam, lParam);
}

BOOL RegisterPopUpTrayHotKey(HWND hTrayWindow, UINT iKeyCombo)
{
    return RegisterHotKey(
        hTrayWindow,
        ID_HOTKEY_EXPAND,
        MOD_CONTROL | MOD_SHIFT,
        iKeyCombo
    );
}

void UnRegisterPopUpTrayHotKey(HWND hTrayWindow)
{
    UnregisterHotKey(hTrayWindow, ID_HOTKEY_EXPAND);
}

BOOL UpdatePopUpTrayHotKey(HWND hTrayWindow, UINT iKeyCombo)
{
    UnRegisterPopUpTrayHotKey(hTrayWindow);

    if (!RegisterPopUpTrayHotKey(hTrayWindow, iKeyCombo))
        return FALSE;

    const std::wstring key = VkToHotkeyString(iKeyCombo);

    return WritePrivateProfileStringW(
        L"settings",
        L"hotkey",
        key.c_str(),
        GetIniPath().c_str()
    );
}
