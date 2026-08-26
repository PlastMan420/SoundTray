#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "AudioControl.h"

constexpr int MAX_LOADSTRING = 100;

// Global Variables:
namespace globals {
    extern HINSTANCE hInst;                                // current instance
    extern WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
    extern WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
    extern WCHAR szTrayWindowClass[MAX_LOADSTRING];      // the tray window class name
    extern HWND hTrayPopup;
    extern HWND hTrayContent;
    extern WASAPIAudioManager sAudioDevices;
}
