#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "AudioControl.h"
#include <unordered_map>
#include <list>
#include <memory>

class ProcessManager {
private:
    std::unordered_map<DWORD, std::unique_ptr<AudioControl>> _gProcessTable = std::unordered_map<DWORD, std::unique_ptr<AudioControl>>();
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
    std::list<std::shared_ptr<WASAPIProcess>> _EnumerateAudioSessions(WASAPIAudioManager& audioMgr);

    /// <summary>
    /// Take a list of enumeratedProcessesList. Clean up processTable from dead processes then add new processes.
    /// </summary>
    /// <param name="enumeratedProcessesList"></param>
    /// <param name="processTable"></param>
    void _UpdateProcessTable(HWND trayWindowHwnd, std::list<std::shared_ptr<WASAPIProcess>>& enumeratedProcessesList, std::unordered_map<DWORD, std::unique_ptr<AudioControl>>& processTable);

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

    /// <summary>
    /// Refersh list of processes and make them available for use in the supplied window handles.
    /// </summary>
    /// <param name="trayWindowHwnd"></param>
    void _UpdateAudioProcessesList(HWND trayWindowHwnd, WASAPIAudioManager& audioDevices);

    void _ArrangeTrayWindowUI(std::unordered_map<DWORD, std::unique_ptr<AudioControl>>& processTable);

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
    std::list<std::shared_ptr<WASAPIProcess>> EnumerateAudioSessions();

    /// <summary>
    /// Refersh list of processes and make them available for use in the supplied window handles.
    /// </summary>
    /// <param name="trayWindowHwnd"></param>
    void UpdateAudioProcessesList(HWND trayWindowHwnd);

    /// <summary>
    /// Trigger a relayout of the tray content using the current process table.
    /// </summary>
    void RelayoutTray();

    void SetContentScroll(HWND contentWindow, int contentHeight);

    void ArrangeTrayWindowUI();

    /// <summary>
    /// Show the tray child window about taskbar trigger.
    /// </summary>
    /// <param name="popup"></param>
    void ShowhTrayPopup(HWND popup);

    void ComputeTrayWindowPositionAndDisplay(HWND popup, int contentWidth, int contentHeight);
};
