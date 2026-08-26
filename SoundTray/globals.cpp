#include "pch.h"
#include "global.h"

namespace globals {
    HINSTANCE hInst = NULL;
    WCHAR szTitle[MAX_LOADSTRING] = L"SoundTray";
    WCHAR szWindowClass[MAX_LOADSTRING] = L"soundtray__mainwindow";
    WCHAR szTrayWindowClass[MAX_LOADSTRING] = L"soundtray__childwindow";
    HWND TrayPopup = NULL;
    HWND TrayContent = NULL;
    WASAPIAudioManager sAudioDevices = {};
}
