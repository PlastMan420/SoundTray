#include "pch.h"
#include <string>

constexpr wchar_t INI_FILE_NAME[] = L"soundtray.ini";
constexpr wchar_t INI_SECTION[] = L"settings";
constexpr wchar_t INI_KEY[] = L"hotkey";

std::wstring GetIniPath()
{
    wchar_t path[MAX_PATH];

    GetModuleFileNameW(
        nullptr,
        path,
        MAX_PATH
    );

    std::wstring result(path);

    const size_t slash = result.find_last_of(L"\\/");

    if (slash != std::wstring::npos)
        result.resize(slash + 1);
    else
        result.clear();

    result += INI_FILE_NAME;

    return result;
}

UINT HotkeyStringToVk(const std::wstring& key)
{
    if (key == L"UP")     return VK_UP;
    if (key == L"DOWN")   return VK_DOWN;
    if (key == L"LEFT")   return VK_LEFT;
    if (key == L"RIGHT")  return VK_RIGHT;
    if (key == L"SPACE")  return VK_SPACE;
    if (key == L"ENTER")  return VK_RETURN;
    if (key == L"TAB")    return VK_TAB;
    if (key == L"ESC")    return VK_ESCAPE;

    // A-Z
    if (key.size() == 1 &&
        key[0] >= L'A' &&
        key[0] <= L'Z')
    {
        return static_cast<UINT>(key[0]);
    }

    return VK_UP;
}

std::wstring VkToHotkeyString(UINT vk)
{
    switch (vk)
    {
    case VK_UP:      return L"UP";
    case VK_DOWN:    return L"DOWN";
    case VK_LEFT:    return L"LEFT";
    case VK_RIGHT:   return L"RIGHT";
    case VK_SPACE:   return L"SPACE";
    case VK_RETURN:  return L"ENTER";
    case VK_TAB:     return L"TAB";
    case VK_ESCAPE:  return L"ESC";
    }

    if (vk >= 'A' && vk <= 'Z')
        return std::wstring(1, static_cast<wchar_t>(vk));

    return L"UP";
}


UINT ConfigLoadHotkey()
{
    wchar_t buffer[64];

    GetPrivateProfileStringW(
        INI_SECTION,
        INI_KEY,
        L"UP",
        buffer,
        ARRAYSIZE(buffer),
        GetIniPath().c_str()
    );

    return HotkeyStringToVk(buffer);
}
