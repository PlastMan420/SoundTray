#include "pch.h"
#include "processmgr.h"
#include <list>
#include <memory>
#include <unordered_map>
#include "AudioControl.h"
#include "global.h"

/// <summary>
/// Store the audio control with its owning process.
/// </summary>
auto _gProcessTable = std::unordered_map<DWORD, std::unique_ptr<AudioControl>>();

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

WASAPIAudioManager CreateAudioDevices() {
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

 void UpdateAudioProcessesList(HWND trayWindowHwnd, WASAPIAudioManager& audioDevices) {
    auto listOfNewProcesses = EnumerateAudioSessions(audioDevices);
    // Debug: if no sessions found, emit a message to help diagnose empty UI
    if (listOfNewProcesses.empty()) {
        wchar_t buf[256];
        bool hasSessionMgr = (audioDevices.sessionManager != nullptr);
        swprintf_s(buf, L"Enumerated 0 audio sessions. sessionManager %s\n", hasSessionMgr ? L"present" : L"null");
        OutputDebugStringW(buf);
        // show a non-modal message so user can see immediate feedback during testing
        MessageBoxW(nullptr, buf, L"SoundTray Debug", MB_OK | MB_ICONINFORMATION);
    }
    UpdateProcessTable(trayWindowHwnd, listOfNewProcesses, _gProcessTable);
}

std::list<std::shared_ptr<WASAPIProcess>> EnumerateAudioSessions(WASAPIAudioManager& audioMgr)
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

void UpdateProcessTable(HWND trayWindowHwnd, std::list<std::shared_ptr<WASAPIProcess>>& enumeratedProcessesList, std::unordered_map<DWORD, std::unique_ptr<AudioControl>>& processTable)
{
    // Scan for dead processes and remove.
    auto it = processTable.begin();
    while (it != processTable.end()) {
        auto isRunning = IsProcessRunningByPID(it->first);
        if (!isRunning) {
            it = processTable.erase(it);
        }
        else {
            ++it;
        }
    }

    // Add new processes.
    for (const auto& item : enumeratedProcessesList) {
        if (!processTable.contains(item->processId)) {
            processTable.emplace(item->processId, std::make_unique<AudioControl>(GetModuleHandle(NULL), globals::hTrayContent, item));
            auto control = processTable.at(item->processId).get();
            control->Draw();
        }
    }

    // Fix layout.
    ArrangeTrayWindowUI(processTable);

    for (const auto& [key, value] : processTable) {
        auto control = value.get();
        control->UpdateUI();
    }
}

inline  bool IsProcessRunningByPID(DWORD pid) {
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

ProcessInfo GetProcessInfo(DWORD pid)
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

void ArrangeTrayWindowUI(std::unordered_map<DWORD, std::unique_ptr<AudioControl>>& processTable)
{
    RECT rc{};
    GetClientRect(globals::hTrayContent, &rc);

    const int margin = 10;
    const int spacing = 8;
    // Ensure control height is large enough to accommodate a vertical slider (min 150) plus labels and mute button
    const int minControlHeight = 10 + 24 + 150 + 6 + 25 + 10; // padding + label + slider + gap + mute + padding
    const int controlHeight = max(CONTROL_HEIGHT, minControlHeight);
    const int maxCols = 6;

    int total = static_cast<int>(processTable.size());
    if (total == 0) {
        // nothing to arrange
        SetContentScroll(globals::hTrayContent, 0);
        return;
    }

    int cols = min(maxCols, total);
    int rows = (total + cols - 1) / cols;

    int availableWidth = rc.right - rc.left - margin * 2;
    int controlWidth = (availableWidth - (cols - 1) * spacing) / cols;

    int index = 0;
    for (auto& [pid, audioControl] : processTable)
    {
        int col = index % cols;
        int row = index / cols;

        int x = margin + col * (controlWidth + spacing);
        int y = margin + row * (controlHeight + spacing);

        audioControl->SetPosition(
            x,
            y,
            controlWidth,
            controlHeight
        );

        ++index;
    }

    int contentHeight = margin + rows * (controlHeight + spacing);
    SetContentScroll(globals::hTrayContent, contentHeight);
}

void SetContentScroll(HWND contentWindow, int contentHeight) {
    RECT rc{};
    int visibleHeight = 0;
    if (contentWindow && GetClientRect(contentWindow, &rc)) {
        visibleHeight = rc.bottom - rc.top;
    }

    if (visibleHeight <= 0)
        visibleHeight = 1;

    SCROLLINFO si{};
    si.cbSize = sizeof(si);
    si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
    si.nMin = 0;
    si.nMax = max(0, contentHeight - 1);
    si.nPage = visibleHeight;

    // preserve existing position if any
    SCROLLINFO old{};
    old.cbSize = sizeof(old);
    old.fMask = SIF_POS;
    if (GetScrollInfo(contentWindow, SB_VERT, &old)) {
        si.nPos = min(old.nPos, max(0, si.nMax - static_cast<int>(si.nPage) + 1));
    } else {
        si.nPos = 0;
    }

    SetScrollInfo(contentWindow, SB_VERT, &si, TRUE);
}

void RelayoutTray() {
    ArrangeTrayWindowUI(_gProcessTable);
}
