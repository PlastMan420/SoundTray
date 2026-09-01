#define NOMINMAX
#include "pch.h"
#include "processmgr.h"
#include <list>

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
    ProcessManager::_sAudioDevice = CreateAudioDevices();
}

std::vector<ProcessInfo> GetProcessListSnapshot()
{
    std::vector<ProcessInfo> snapshot;
    WASAPIAudioManager mgr = CreateAudioDevices();
    auto sessions = ProcessManager::EnumerateAudioSessions(mgr);
    for (auto& procPtr : sessions) {
        snapshot.push_back(procPtr->sProcessInfo);
    }

    return snapshot;
}

// C wrapper: allocates an array of ProcessInfo via new[]; caller must delete[] after use
size_t GetProcessListSnapshot_C(ProcessInfo** outArray)
{
    if (!outArray) return 0;
    auto snap = GetProcessListSnapshot();
    size_t n = snap.size();
    if (n == 0) {
        *outArray = nullptr;
        return 0;
    }

// Implementation of ProcessManager facade
std::list<std::shared_ptr<WASAPIProcess>> ProcessManager::EnumerateAudioSessions(WASAPIAudioManager& mgr)
{
    return EnumerateAudioSessions(mgr);
}

    ProcessInfo* arr = new ProcessInfo[n];
    for (size_t i = 0; i < n; ++i) arr[i] = snap[i];
    *outArray = arr;
    return n;
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

std::list<WASAPIProcess> ProcessManager::EnumerateAudioSessions() {
    return ProcessManager::_EnumerateAudioSessions(this->_sAudioDevice);
}

inline std::list<WASAPIProcess> ProcessManager::_EnumerateAudioSessions(WASAPIAudioManager& audioMgr)
{
    Microsoft::WRL::ComPtr<IAudioSessionEnumerator> sessions;
    audioMgr.sessionManager->GetSessionEnumerator(&sessions);

    int count = 0;
    sessions->GetCount(&count);

    std::list<WASAPIProcess> processes;

    for (int i = 0; i < count; ++i)
    {
        Microsoft::WRL::ComPtr<IAudioSessionControl> control;
        sessions->GetSession(i, &control);

        // Elevate to IAudioSessionControl2. requires Windows 7
        Microsoft::WRL::ComPtr<IAudioSessionControl2> control2;
        control.As(&control2);

        DWORD processId = 0;
        control2->GetProcessId(&processId);

        // Omit IDLE process.
        if (processId == 0) {
            continue;
        }

        Microsoft::WRL::ComPtr<ISimpleAudioVolume> volume;
        control.As(&volume);

        float level = 0.0f;
        volume->GetMasterVolume(&level);

        // processId = application process
        // level     = 0.0f - 1.0f
        auto process = WASAPIProcess(processId, volume, GetProcessInfo(processId));
        processes.push_back(process);
    }

    return processes;
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
            sProcessInfo.szProcessName = fileInfo.szDisplayName;
            sProcessInfo.hProcessIcon = fileInfo.hIcon;
        }
    }

    CloseHandle(process);

    return sProcessInfo;
}

void ProcessManager::ShowhTrayPopup(HWND popup)
{
    // Fetch current sessions and log count for UI to consume
    try {
        auto list = ProcessManager::EnumerateAudioSessions(globals::sAudioDevices);
        wchar_t buf[256];
        swprintf_s(buf, L"ShowhTrayPopup: session count=%u\n", static_cast<unsigned int>(list.size()));
        OutputDebugStringW(buf);

        // Log each PID
        for (const auto& p : list) {
            swprintf_s(buf, L"  pid=%lu\n", p->processId);
            OutputDebugStringW(buf);
        }
    }
    catch (...) {
        OutputDebugStringW(L"ShowhTrayPopup: EnumerateAudioSessions threw\n");
    }

    // Activate window
    SetForegroundWindow(popup);
}
