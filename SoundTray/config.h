#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <string>

std::wstring GetIniPath();

/// <summary>
/// Convert string to hotkey macro for reading.
/// </summary>
/// <param name="vk"></param>
/// <returns></returns>
UINT HotkeyStringToVk(const std::wstring& key);

/// <summary>
/// Convert hotkey macro to string for saving.
/// </summary>
/// <param name="vk"></param>
/// <returns></returns>
std::wstring VkToHotkeyString(UINT vk);

/// <summary>
/// Get config file path.
/// </summary>
/// <returns></returns>
std::wstring GetIniPath();

/// <summary>
/// Load hotket option.
/// </summary>
/// <returns></returns>
UINT ConfigLoadHotkey();
