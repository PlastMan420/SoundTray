#define NOMINMAX
#include "pch.h"
#include "processmgr.h"
#include <list>
#include <memory>
#include <unordered_map>
#include "AudioControl.h"
#include "global.h"
#include <algorithm>

/*
IMMDeviceEnumerator
        ↓
Default render device
        ↓
IAudioSessionManager2
        ↓
IAudioSessionEnumerator
        ↓
IAudioSessionControl2
        ↓
Process ID
        ↓
ISimpleAudioVolume
        ↓
Volume (0.0f – 1.0f)
*/

ProcessManager::ProcessManager()
{
    ProcessManager::_gProcessTable = std::unordered_map<DWORD, std::unique_ptr<AudioControl>>();
    ProcessManager::_sAudioDevice = CreateAudioDevices();
}

const WASAPIAudioManager& ProcessManager::GetAudioDevices() const
{
    return ProcessManager::_sAudioDevice;
}

WASAPIAudioManager ProcessManager::CreateAudioDevices() {
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    Microsoft::WRL::ComPtr<IMMDeviceEnumerator> deviceEnumerator;

    if (CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        nullptr,
        CLSCTX_ALL,
        IID_PPV_ARGS(&deviceEnumerator)
    ) != S_OK) {
        auto error = GetLastError();
        error = error;
    }

    Microsoft::WRL::ComPtr<IMMDevice> device;

    deviceEnumerator->GetDefaultAudioEndpoint(
        eRender,
        eMultimedia,
        &device
    );

    Microsoft::WRL::ComPtr<IAudioSessionManager2> sessionManager;

    device->Activate(
        __uuidof(IAudioSessionManager2),
        CLSCTX_ALL,
        nullptr,
        reinterpret_cast<void**>(sessionManager.GetAddressOf())
    );

    return WASAPIAudioManager(deviceEnumerator, device, sessionManager);
}

void ProcessManager::UpdateAudioProcessesList(HWND trayWindowHwnd) {
    this->_UpdateAudioProcessesList(trayWindowHwnd, this->_sAudioDevice);
}

inline void ProcessManager::_UpdateAudioProcessesList(HWND trayWindowHwnd, WASAPIAudioManager& audioDevices) {
    auto listOfNewProcesses = _EnumerateAudioSessions(audioDevices);
    
    if (listOfNewProcesses.empty()) {
        wchar_t buf[256];
        bool hasSessionMgr = (audioDevices.sessionManager != nullptr);
        swprintf_s(buf, L"Enumerated 0 audio sessions. sessionManager %s\n", hasSessionMgr ? L"present" : L"null");
        OutputDebugStringW(buf);
    }
    _UpdateProcessTable(trayWindowHwnd, listOfNewProcesses, _gProcessTable);
}

std::list<std::shared_ptr<WASAPIProcess>> ProcessManager::EnumerateAudioSessions() {
    return ProcessManager::_EnumerateAudioSessions(this->_sAudioDevice);
}

inline std::list<std::shared_ptr<WASAPIProcess>> ProcessManager::_EnumerateAudioSessions(WASAPIAudioManager& audioMgr)
{
    Microsoft::WRL::ComPtr<IAudioSessionEnumerator> sessions;
    audioMgr.sessionManager->GetSessionEnumerator(&sessions);

    int count = 0;
    sessions->GetCount(&count);

    std::list<std::shared_ptr<WASAPIProcess>> processes;

    for (int i = 0; i < count; ++i)
    {
        Microsoft::WRL::ComPtr<IAudioSessionControl> control;
        sessions->GetSession(i, &control);

        // Elevate to IAudioSessionControl2. requires Windows 7
        Microsoft::WRL::ComPtr<IAudioSessionControl2> control2;
        control.As(&control2);

        DWORD processId = 0;
        control2->GetProcessId(&processId);

        Microsoft::WRL::ComPtr<ISimpleAudioVolume> volume;
        control.As(&volume);

        float level = 0.0f;
        volume->GetMasterVolume(&level);

        // processId = application process
        // level     = 0.0f - 1.0f
        auto process = std::make_shared<WASAPIProcess>(processId, volume);
        process->sProcessInfo = GetProcessInfo(processId);
        processes.push_back((process));
    }

    return processes;
}

void ProcessManager::_UpdateProcessTable(HWND trayWindowHwnd, std::list<std::shared_ptr<WASAPIProcess>>& enumeratedProcessesList, std::unordered_map<DWORD, std::unique_ptr<AudioControl>>& processTable)
{
    // Scan for dead processes and remove.
    auto it = processTable.begin();
    while (it != processTable.end()) {
        auto isRunning = _IsProcessRunningByPID(it->first);
        if (!isRunning) {
            it = processTable.erase(it);
        }
        else {
            ++it;
        }
    }

    // Add new processes.
    for (const auto& item : enumeratedProcessesList) {
        // Filter out IDLE process
        auto pid = item->processId;
        if (item->processId == 0) {
            continue;
        }

        if (!processTable.contains(item->processId)) {
            processTable.emplace(item->processId, std::make_unique<AudioControl>(GetModuleHandle(NULL), globals::hTrayContent, item));
            auto control = processTable.at(item->processId).get();
            control->Draw();
        }
    }

    // Fix layout.
    _ArrangeTrayWindowUI(processTable);

    for (const auto& [key, value] : processTable) {
        auto control = value.get();
        control->UpdateUI();
    }
}

inline bool ProcessManager::_IsProcessRunningByPID(DWORD pid) {
    // 1. Open a handle with specific access rights
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);

    // If handle is NULL, check why it failed
    if (hProcess == NULL) {
        DWORD error = GetLastError();
        // If access is denied, the process exists but you don't have privileges
        if (error == ERROR_ACCESS_DENIED) {
            return true;
        }
        // Usually ERROR_INVALID_PARAMETER (87) means the PID does not exist
        return false;
    }

    // 2. Check the execution status
    DWORD exitCode;
    bool isRunning = false;
    if (GetExitCodeProcess(hProcess, &exitCode)) {
        isRunning = (exitCode == STILL_ACTIVE);
    }

    // 3. Always clean up the handle
    CloseHandle(hProcess);
    return isRunning;
}

ProcessInfo ProcessManager::GetProcessInfo(DWORD pid)
{
    ProcessInfo sProcessInfo{};

    HANDLE process = OpenProcess(
        PROCESS_QUERY_LIMITED_INFORMATION,
        FALSE,
        pid
    );

    if (!process)
        return sProcessInfo;

    wchar_t path[MAX_PATH];
    DWORD size = MAX_PATH;

    if (QueryFullProcessImageNameW(
        process,
        0,
        path,
        &size))
    {
        SHFILEINFOW fileInfo{};

        if (SHGetFileInfoW(
            path,
            0,
            &fileInfo,
            sizeof(fileInfo),
            SHGFI_DISPLAYNAME | SHGFI_ICON | SHGFI_SMALLICON))
        {
            sProcessInfo.hProcessName = fileInfo.szDisplayName;
            sProcessInfo.hProcessIcon = fileInfo.hIcon;
        }
    }

    CloseHandle(process);

    return sProcessInfo;
}

void ProcessManager::ArrangeTrayWindowUI() {
    this->_ArrangeTrayWindowUI(this->_gProcessTable);
}

inline void ProcessManager::_ArrangeTrayWindowUI(
    std::unordered_map<DWORD, std::unique_ptr<AudioControl>>& processTable)
{
    constexpr int margin = 10;
    constexpr int spacing = 8;
    constexpr int controlWidth = 40;
    constexpr int controlHeight = 225;
    constexpr int maxCols = 4;

    const int total = static_cast<int>(processTable.size());

    const int cols = std::clamp(1, total, maxCols);
    const int rows = std::max(1, ((total + cols - 1) / cols));

    const int trayWidth =
        margin * 2 +
        cols * controlWidth +
        (cols - 1) * spacing;

    const int trayHeight =
        margin * 2 +
        rows * controlHeight +
        (rows - 1) * spacing;

    // Resize popup/content window.
    ComputeTrayWindowPositionAndDisplay(globals::hTrayPopup, trayWidth, trayHeight);

    // Resize the content area too if you're using one.
    SetWindowPos(
        globals::hTrayContent,
        nullptr,
        0, 0,
        trayWidth,
        trayHeight,
        SWP_NOZORDER |
        SWP_NOACTIVATE
    );

    int index = 0;

    for (auto& [pid, audioControl] : processTable)
    {
        const int col = index % cols;
        const int row = index / cols;

        const int x =
            margin + col * (controlWidth + spacing);

        const int y =
            margin + row * (controlHeight + spacing);

        audioControl->SetPosition(
            x,
            y,
            controlWidth,
            controlHeight
        );

        ++index;
    }

    SetContentScroll(
        globals::hTrayContent,
        trayHeight
    );
}

void ProcessManager::SetContentScroll(HWND contentWindow, int contentHeight) {
    RECT rc{};
    GetClientRect(contentWindow, &rc);
    LONG visibleHeight = std::max<LONG>(1, (rc.bottom - rc.top));

    SCROLLINFO si{};
    si.cbSize = sizeof(si);
    si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
    si.nMin = 0;
    si.nMax = std::max(0, contentHeight - 1);
    si.nPage = visibleHeight;

    // preserve existing position if any
    SCROLLINFO old{};
    old.cbSize = sizeof(old);
    old.fMask = SIF_POS;
    if (GetScrollInfo(contentWindow, SB_VERT, &old)) {
        si.nPos = std::min(old.nPos, std::max(0, si.nMax - static_cast<int>(si.nPage) + 1));
    } else {
        si.nPos = 0;
    }

    SetScrollInfo(contentWindow, SB_VERT, &si, TRUE);
}

void ProcessManager::RelayoutTray() {
    ProcessManager::_ArrangeTrayWindowUI(this->_gProcessTable);
}

void ProcessManager::ShowhTrayPopup(HWND popup)
{
    // Activate window
    SetForegroundWindow(popup);
}

void ProcessManager::ComputeTrayWindowPositionAndDisplay(HWND popup, int contentWidth, int contentHeight) {
    RECT taskbar{};

    HWND taskbarWindow = FindWindowW(L"Shell_TrayWnd", nullptr);

    if (!taskbarWindow)
        return;

    GetWindowRect(taskbarWindow, &taskbar);

    int x = taskbar.right - contentWidth;
    int y = taskbar.top - contentHeight;

    SetWindowPos(
        popup,
        HWND_TOPMOST,
        x,
        y,
        contentWidth,
        contentHeight,
        SWP_SHOWWINDOW | SWP_NOACTIVATE
    );
}
