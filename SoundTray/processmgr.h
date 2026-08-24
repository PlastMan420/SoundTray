#pragma once
#include "AudioControl.h"
#include <unordered_map>
#include <list>
#include <memory>

/// <summary>
/// Init audio device used for process enumeration.
/// </summary>
/// <returns></returns>
WASAPIAudioManager CreateAudioDevices();

/// <summary>
/// Get list of active processes that use WASAPI.
/// </summary>
/// <param name="audioMgr"></param>
/// <returns>ProcessId and IAudioSessionManager2</returns>
std::list<std::shared_ptr<WASAPIProcess>> EnumerateAudioSessions(WASAPIAudioManager& audioMgr);

/// <summary>
/// Take a list of enumeratedProcessesList. Clean up processTable from dead processes then add new processes.
/// </summary>
/// <param name="enumeratedProcessesList"></param>
/// <param name="processTable"></param>
void UpdateProcessTable(std::list<std::shared_ptr<WASAPIProcess>>& enumeratedProcessesList, std::unordered_map<DWORD, std::unique_ptr<AudioControl>>& processTable);

/// <summary>
/// Check if process is still alive.
/// </summary>
/// <param name="pid"></param>
/// <returns></returns>
inline  bool IsProcessRunningByPID(DWORD pid);

void UpdateAudioProcessesList(WASAPIAudioManager& audioDevices, std::unordered_map<DWORD, std::unique_ptr<AudioControl>>& processTable);
