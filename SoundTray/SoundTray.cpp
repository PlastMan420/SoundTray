// SoundTray.cpp : Defines the entry point for the application.

// Check https://github.com/microsoft/WindowsAppSDK-Samples/blob/main/Samples/Islands/SimpleIslandApp/cpp-win32-unpackaged

#include "pch.h"
#include "SoundTray.h"
#include "AudioControl.h"
#include <crtdbg.h>
#include "global.h"
#include "Resource.h"
#include "hot_key_dialog.h"
#include "config.h"
#include <Microsoft.UI.Dispatching.Interop.h> // For ContentPreTranslateMessage
#include "App.xaml.h"
#include "WinUIHost.h"
#include "winrt/impl/Microsoft.UI.Dispatching.2.h"

namespace winrt
{
    using namespace winrt::Microsoft::UI;
    using namespace winrt::Microsoft::UI::Dispatching;
    using namespace winrt::Microsoft::UI::Xaml;
    using namespace winrt::Microsoft::UI::Xaml::Hosting;
    using namespace winrt::Microsoft::UI::Xaml::Markup;
}

// Extra state for our top-level window, we point to from GWLP_USERDATA.
struct WindowInfo
{
    winrt::DesktopWindowXamlSource DesktopWindowXamlSource{ nullptr };
    winrt::event_token TakeFocusRequestedToken{};
    HWND LastFocusedWindow{ NULL };
};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Enable CRT leak checking in debug builds
#if defined(_DEBUG)
    int tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;
    _CrtSetDbgFlag(tmpDbgFlag);
#endif

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, globals::szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SOUNDTRAY, globals::szWindowClass, MAX_LOADSTRING);

    RegisterMainWindow(hInstance);

    // Init WIN32 common controls and register content/audio classes before creating instances
    //INITCOMMONCONTROLSEX icc{
    //    sizeof(INITCOMMONCONTROLSEX),
    //    ICC_BAR_CLASSES
    //};

    //InitCommonControlsEx(&icc);

    RegisterHotkeyWindowClass(hInstance);

    // Island-support: Call init_apartment to initialize COM and WinRT for the thread.
    winrt::init_apartment(winrt::apartment_type::single_threaded);
    // Initialize WinUI host (if available)
    winui::InitializeWinUI();

    // Perform application initialization:
    auto hMainWindow = InitInstance (hInstance, nCmdShow);

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SOUNDTRAY));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            continue;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Clean up COM
    //CoUninitialize();

    // Remove tray icon
    if (globals::g_nid.cbSize != 0) {
        Shell_NotifyIcon(NIM_DELETE, &globals::g_nid);
    }

    return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM RegisterMainWindow(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SOUNDTRAY));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SOUNDTRAY);
    wcex.lpszClassName  = globals::szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

// Content window for holding audio control child windows
LRESULT CALLBACK ContentWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_VSCROLL:
    {
        SCROLLINFO si{};
        si.cbSize = sizeof(si);
        si.fMask = SIF_ALL;

        GetScrollInfo(hwnd, SB_VERT, &si);

        int oldPos = si.nPos;
        int newPos = oldPos;

        switch (LOWORD(wParam))
        {
        case SB_LINEUP: newPos -= 30; break;
        case SB_LINEDOWN: newPos += 30; break;
        case SB_PAGEUP: newPos -= (int)si.nPage; break;
        case SB_PAGEDOWN: newPos += (int)si.nPage; break;
        case SB_THUMBTRACK: newPos = si.nTrackPos; break;
        }

        newPos = max(0, min(newPos, si.nMax - static_cast<int>(si.nPage) + 1));

        if (newPos != oldPos)
        {
            si.fMask = SIF_POS;
            si.nPos = newPos;
            SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
            ScrollWindowEx(hwnd, 0, oldPos - newPos, nullptr, nullptr, nullptr, nullptr, SW_SCROLLCHILDREN | SW_INVALIDATE);
        }

        return 0;
    }
    case WM_MOUSEWHEEL:
    {
        int delta = GET_WHEEL_DELTA_WPARAM(wParam);
        SendMessageW(hwnd, WM_VSCROLL, delta > 0 ? SB_LINEUP : SB_LINEDOWN, 0);
        return 0;
    }
    default:
        return DefWindowProcW(hwnd, message, wParam, lParam);
    }
}

/// <summary>
/// Init main window and tray icon
/// </summary>
/// <param name="hInstance"></param>
/// <param name="nCmdShow"></param>
/// <returns></returns>
HWND InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   globals::hInst = hInstance; // Store instance handle in our global variable

   // hidden main window
   HWND hWnd = CreateWindowEx(
       0,
       L"SoundTray",
       L"",
       0,
       0, 0, 0, 0,
       HWND_MESSAGE,
       nullptr,
       hInstance,
       nullptr
   );

   if (!hWnd)
   {
      return FALSE;
   }

   globals::hMainWindow = hWnd;

   UINT hotkey = LoadHotkey();
   if (!RegisterPopUpTrayHotKey(hWnd, hotkey)) {
       return FALSE;
   }

   InitTrayIcon(hWnd, hInstance);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return hWnd;
}

UINT LoadHotkey()
{
    wchar_t buffer[64];

    DWORD length = GetPrivateProfileStringW(
        L"settings",
        L"hotkey",
        nullptr,
        buffer,
        ARRAYSIZE(buffer),
        GetIniPath().c_str()
    );

    if (length == 0)
        return VK_UP;

    return HotkeyStringToVk(buffer);
}

void InitTrayIcon(HWND hWnd, HINSTANCE hInstance) {
    NOTIFYICONDATA nid{};
    nid.cbSize = sizeof(nid);
    nid.hWnd = hWnd;

    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP | NIF_GUID;
    nid.uCallbackMessage = WM_APP + 1;

    nid.guidItem = TASKBAR_TRAY_UID;

    nid.dwInfoFlags = NIIF_USER | NIIF_LARGE_ICON;
    nid.uCallbackMessage = WM_MOUSEACTIVATE;

    nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SOUNDTRAY));
    wcscpy_s(nid.szTip, L"My Application");
    // Store globally so we can delete on shutdown
    globals::g_nid = nid;
    Shell_NotifyIcon(NIM_ADD, &globals::g_nid);

    globals::g_nid.uVersion = NOTIFYICON_VERSION_4;
    Shell_NotifyIcon(NIM_SETVERSION, &globals::g_nid);
}

HWND CreatehTrayPopup(HINSTANCE hInstance, HWND owner)
{
    HWND popup = CreateWindowExW(
        WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
        globals::szTrayWindowClass,
        nullptr,
        WS_POPUP,
        0, 0,
        500, 300,
        owner,
        nullptr,
        hInstance,
        nullptr
    );

    if (!popup)
        return nullptr;

    // create content child window (scrollable)
    HWND content = CreateWindowExW(
        0,
        L"SoundhTrayContent",
        nullptr,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL,
        0,
        40,
        500,
        260,
        popup,
        nullptr,
        hInstance,
        nullptr
    );

    globals::hTrayContent = content;

    // If WinUI is available, show WinUI window instead of classic popup
    winui::ShowTrayWindow();

    return popup;
}



/// <summary>
/// Main window WndProc. main window is always hidden.
/// </summary>
/// <param name="hWnd"></param>
/// <param name="message"></param>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <returns></returns>
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_MOUSEACTIVATE:
        {
            if (LOWORD(lParam) == WM_LBUTTONUP)
            {
                //globals::cProcessManager.ShowhTrayPopup(globals::hTrayPopup);
            }

            break;
        }
        case WM_HOTKEY:
            if (wParam == ID_HOTKEY_EXPAND)
            {
                //globals::cProcessManager.ShowhTrayPopup(globals::hTrayPopup);
            }
            return 0;
        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                BeginPaint(hWnd, &ps);
                // TODO: Add any drawing code here...
                EndPaint(hWnd, &ps);
            }
            break;
        case WM_DESTROY:
            // remove tray icon immediately
            if (globals::g_nid.cbSize != 0) {
                Shell_NotifyIcon(NIM_DELETE, &globals::g_nid);
                globals::g_nid = {};
            }
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

/// <summary>
/// Tray window WndProc
/// </summary>
/// <param name="hwnd"></param>
/// <param name="msg"></param>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <returns></returns>
LRESULT CALLBACK PopupWndProc(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (message)
    {
        case WM_CREATE:
        {
            globals::popupXamlSource =
                winrt::Microsoft::UI::Xaml::Hosting::DesktopWindowXamlSource{};

            return 0;
        }
        case WM_NCDESTROY:
        {
            globals::popupXamlSource = nullptr;
            return DefWindowProcW(hWnd, message, wParam, lParam);
        }
        case WM_ACTIVATE:
        {
            //globals::cProcessManager.UpdateAudioProcessesList(hWnd);

            if (LOWORD(wParam) == WA_INACTIVE)
            {
                ShowWindow(hWnd, SW_HIDE);
            }
            break;
        }
        case WM_SIZE:
        {
            // Resize the content child to fit below a top bar of 40 pixels
            if (globals::hTrayContent)
            {
                RECT rc;
                GetClientRect(hWnd, &rc);
                int width = rc.right - rc.left;
                int height = rc.bottom - rc.top;
                // content area starts at y=40
                SetWindowPos(globals::hTrayContent, nullptr, 0, 40, width, (height - 40 > 0 ? height - 40 : 0), SWP_NOZORDER);
                // relayout children to new sizes/columns
                //globals::cProcessManager.RelayoutTray();
            }
            break;
        }
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code here...
            EndPaint(hWnd, &ps);
            break;
        }
        case WM_VSCROLL:
        {
            HWND slider = reinterpret_cast<HWND>(lParam);

            auto* control = reinterpret_cast<AudioControl*>(
                GetWindowLongPtrW(slider, GWLP_USERDATA)
                );

            if (control)
            {
                int position = static_cast<int>(
                    SendMessageW(slider, TBM_GETPOS, 0, 0)
                    );

                // WIN32 sliders have 0 at top.
                float volume = 1.0f - (position / 100.0f);
                control->SetVolume(volume);
            }

            return 0;
        }
        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(globals::hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_HOTKEY:
                CreateHotkeyWindow(globals::hInst);
                break;
            case IDM_EXIT:
                
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }


    return DefWindowProcW(hWnd, message, wParam, lParam);
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// Returns "true" if the function handled the message and it shouldn't be processed any further.
// Intended to be called from the main message loop.
bool ProcessMessageForTabNavigation(const HWND topLevelWindow, MSG* msg)
{
    if (msg->message == WM_KEYDOWN && msg->wParam == VK_TAB)
    {
        // The user is pressing the "tab" key.  We want to handle this ourselves so we can pass information into Xaml
        // about the tab navigation.  Specifically, we need to tell Xaml whether this is a forward tab, or a backward
        // shift+tab, so Xaml will know whether to put focus on the first Xaml element in the island or the last
        // Xaml element.  (This is done in the call to DesktopWindowXamlSource.NavigateFocus()).
        const HWND currentFocusedWindow = ::GetFocus();
        if (::GetAncestor(currentFocusedWindow, GA_ROOT) != topLevelWindow)
        {
            // This is a window outside of our top-level window, let the system process it.
            return false;
        }

        const bool isShiftKeyDown = ((HIWORD(::GetKeyState(VK_SHIFT)) & 0x8000) != 0);
        const HWND nextFocusedWindow = ::GetNextDlgTabItem(topLevelWindow, currentFocusedWindow, isShiftKeyDown /*bPrevious*/);

        WindowInfo* windowInfo = reinterpret_cast<WindowInfo*>(::GetWindowLongPtr(topLevelWindow, GWLP_USERDATA));
        const HWND dwxsWindow = winrt::GetWindowFromWindowId(windowInfo->DesktopWindowXamlSource.SiteBridge().WindowId());
        if (dwxsWindow == nextFocusedWindow)
        {
            // Focus is moving to our DesktopWindowXamlSource.  Instead of just calling SetFocus on it, we call NavigateFocus(),
            // which allows us to tell Xaml which direction the keyboard focus is moving.
            // If your app has multiple DesktopWindowXamlSources in the window, you'll want to loop over them and check to
            // see if focus is moving to each one.
            winrt::XamlSourceFocusNavigationRequest request{
                isShiftKeyDown ?
                    winrt::XamlSourceFocusNavigationReason::Last :
                    winrt::XamlSourceFocusNavigationReason::First };

            windowInfo->DesktopWindowXamlSource.NavigateFocus(request);
            return true;
        }

        // Focus isn't moving to our DesktopWindowXamlSource.  IsDialogMessage will automatically do the tab navigation
        // for us for this msg.
        const bool handled = (::IsDialogMessage(topLevelWindow, msg) == TRUE);
        return handled;
    }
    return false;
}
