#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <list>
#include <wrl/client.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>

struct WASAPIAudioManager {
    Microsoft::WRL::ComPtr<IMMDeviceEnumerator> deviceEnumerator;
    Microsoft::WRL::ComPtr<IMMDevice> device;
    Microsoft::WRL::ComPtr<IAudioSessionManager2> sessionManager;
};

class ProcessInfo
{
public:
    ProcessInfo(){}
    ProcessInfo(std::wstring szProcessName, HICON hProcessIcon) : szProcessName(szProcessName), hProcessIcon(hProcessIcon) {}
    ~ProcessInfo(){
        DestroyIcon(hProcessIcon);
    }
    std::wstring szProcessName;
    HICON hProcessIcon{};
};

class WASAPIProcess {
public:
    WASAPIProcess(){}
    WASAPIProcess(DWORD processId, Microsoft::WRL::ComPtr<ISimpleAudioVolume> volumeControl, ProcessInfo pcProcessInfo)
    : processId(processId), volumeControl(volumeControl), pcProcessInfo(pcProcessInfo) {}
    DWORD processId;
    Microsoft::WRL::ComPtr<ISimpleAudioVolume> volumeControl;
    ProcessInfo pcProcessInfo;
};

class ProcessManager {
private:
    WASAPIAudioManager _sAudioDevice;

    /// <summary>
    /// Init audio device used for process enumeration.
    /// </summary>
    /// <returns></returns>
    WASAPIAudioManager CreateAudioDevices();

    /// <summary>
    /// Get list of active processes that use WASAPI. Private version takes an input to make testing easier.
    /// </summary>
    /// <param name="audioMgr"></param>
    /// <returns>ProcessId and IAudioSessionManager2</returns>
    std::list<WASAPIProcess> _EnumerateAudioSessions(WASAPIAudioManager& audioMgr);

    /// <summary>
    /// Check if process is still alive.
    /// </summary>
    /// <param name="pid"></param>
    /// <returns></returns>
    bool _IsProcessRunningByPID(DWORD pid);

    /// <summary>
    /// Returns name and icon of a process in a struct.
    /// </summary>
    /// <param name="pid"></param>
    /// <returns></returns>
    ProcessInfo GetProcessInfo(DWORD pid);

// Return a snapshot of current audio processes (lightweight copy) for UI consumption.
// Caller takes ownership of returned vector.
std::vector<ProcessInfo> GetProcessListSnapshot();

// Provide a ProcessManager facade so callers (e.g., UI) can call EnumerateAudioSessions via a class method.
class ProcessManager {
public:
    // Returns a list of WASAPIProcess shared_ptrs representing current audio sessions.
    static std::list<std::shared_ptr<WASAPIProcess>> EnumerateAudioSessions(WASAPIAudioManager& mgr);
};

extern "C" {
    // C wrapper for interop: returns number of entries; out pointer will be allocated by caller
    size_t GetProcessListSnapshot_C(ProcessInfo** outArray);
}

    /// <summary>
    /// Refersh list of processes and make them available for use in the supplied window handles.
    /// </summary>
    /// <param name="trayWindowHwnd"></param>

public:
    ProcessManager();

    /// <summary>
    /// Get reference to audio manager struct.
    /// </summary>
    /// <returns></returns>
    const WASAPIAudioManager& GetAudioDevices() const;


    /// <summary>
    /// Get list of active processes that use WASAPI.
    /// </summary>
    /// <param name="audioMgr"></param>
    /// <returns>ProcessId and IAudioSessionManager2</returns>
    std::list<WASAPIProcess> EnumerateAudioSessions();

    /// <summary>
    /// Show the tray child window about taskbar trigger.
    /// </summary>
    /// <param name="popup"></param>
    void ShowhTrayPopup(HWND popup);
};
