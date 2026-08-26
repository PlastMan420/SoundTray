// SoundTray.cpp : Defines the entry point for the application.
//

#include "pch.h"
#include "SoundTray.h"
#include "AudioControl.h"
#include "processmgr.h"
#include "global.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, globals::szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SOUNDTRAY, globals::szWindowClass, MAX_LOADSTRING);

    RegisterMainWindow(hInstance);

    // Init WIN32 common controls and register content/audio classes before creating instances
    INITCOMMONCONTROLSEX icc{
        sizeof(INITCOMMONCONTROLSEX),
        ICC_BAR_CLASSES
    };

    InitCommonControlsEx(&icc);

    RegisterContentWindow(hInstance);
    AudioControl::RegisterAudioControlWindow();

    globals::sAudioDevices = CreateAudioDevices();

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SOUNDTRAY));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
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

ATOM RegisterContentWindow(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex{};
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = ContentWndProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wcex.lpszClassName = L"SoundhTrayContent";
    return RegisterClassExW(&wcex);
}

ATOM RegisterTrayWindow(HINSTANCE hInstance) {
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = PopupWndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SOUNDTRAY));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = CreateSolidBrush(RGB(192, 192, 192));
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_SOUNDTRAY);
    wcex.lpszClassName = globals::szTrayWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    
    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   globals::hInst = hInstance; // Store instance handle in our global variable

   // hidden window
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

   InitTrayIcon(hWnd, hInstance);

   RegisterTrayWindow(hInstance);
   globals::hTrayPopup = CreatehTrayPopup(hInstance, hWnd);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

void InitTrayIcon(HWND hWnd, HINSTANCE hInstance) {
    NOTIFYICONDATA nid{};
    nid.cbSize = sizeof(nid);
    nid.hWnd = hWnd;

    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP | NIF_GUID;
    nid.uCallbackMessage = WM_APP + 1;

    nid.guidItem = {
        0x12345678,
        0x1234,
        0x5678,
        { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0 }
    };

    nid.dwInfoFlags = NIIF_USER | NIIF_LARGE_ICON;
    nid.uCallbackMessage = WM_MOUSEACTIVATE;

    nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SOUNDTRAY));
    wcscpy_s(nid.szTip, L"My Application");

    Shell_NotifyIcon(NIM_ADD, &nid);

    nid.uVersion = NOTIFYICON_VERSION_4;
    Shell_NotifyIcon(NIM_SETVERSION, &nid);
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

    return popup;
}

void ShowhTrayPopup(HWND popup)
{
    RECT taskbar{};

    HWND taskbarWindow = FindWindowW(L"Shell_TrayWnd", nullptr);

    if (!taskbarWindow)
        return;

    GetWindowRect(taskbarWindow, &taskbar);

    constexpr int width = 500;
    constexpr int height = 300;

    int x = taskbar.right - width;
    int y = taskbar.top - height;

    SetWindowPos(
        popup,
        HWND_TOPMOST,
        x,
        y,
        width,
        height,
        SWP_SHOWWINDOW | SWP_NOACTIVATE
    );

    // Activate window
    SetForegroundWindow(popup);
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
                ShowhTrayPopup(globals::hTrayPopup);
            }

            break;
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
                case IDM_EXIT:
                    DestroyWindow(hWnd);
                    break;
                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
                }
            }
            break;
        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                BeginPaint(hWnd, &ps);
                // TODO: Add any drawing code here...
                EndPaint(hWnd, &ps);
            }
            break;
        case WM_DESTROY:
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
        case WM_ACTIVATE:
        {
            UpdateAudioProcessesList(hWnd, globals::sAudioDevices);

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
                RelayoutTray();
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
