#include "pch.h"
#include "processmgr.h"
#include <list>
#include <memory>
#include <unordered_map>
#include "AudioControl.h"

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
            processTable.emplace(item->processId, std::make_unique<AudioControl>(item));
            auto control = processTable.at(item->processId).get();
            control->Draw(trayWindowHwnd);
        }
    }

    // Fix layout.
    AudioControl::LayoutAudioControls(processTable);

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
            sProcessInfo.processName = fileInfo.szDisplayName;
            sProcessInfo.processIcon = fileInfo.hIcon;
        }
    }

    CloseHandle(process);

    return sProcessInfo;
}
