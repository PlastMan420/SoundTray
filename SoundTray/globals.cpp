#include "pch.h"
#include "global.h"

namespace globals {
    HINSTANCE hInst = NULL;

    WCHAR szTitle[MAX_LOADSTRING] = L"SoundTray";
    WCHAR szWindowClass[MAX_LOADSTRING] = L"soundtray__mainwindow";
    WCHAR szTrayWindowClass[MAX_LOADSTRING] = L"soundtray__childwindow";
    WCHAR szHotKeyWindowClass[MAX_LOADSTRING] = L"soundtray__HotkeyWindow";
    
    HWND hTrayPopup = NULL;
    HWND hMainWindow = NULL;
    HWND hTrayContent = NULL;

    WASAPIAudioManager sAudioDevices = {};
    NOTIFYICONDATA g_nid = {};
    UINT iTrayHotKey = VK_UP;
}
