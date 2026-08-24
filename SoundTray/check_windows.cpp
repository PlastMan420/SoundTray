#include "pch.h"
#include "check_windows.h"

bool CheckWindows(int ver)
{
    OSVERSIONINFOEXW osvi{};
    osvi.dwOSVersionInfoSize = sizeof(osvi);

    osvi.dwMajorVersion = ver / 10;
    osvi.dwMinorVersion = ver % 10;

    DWORDLONG conditionMask = 0;
    VER_SET_CONDITION(conditionMask, VER_MAJORVERSION, VER_EQUAL);
    VER_SET_CONDITION(conditionMask, VER_MINORVERSION, VER_EQUAL);

    return VerifyVersionInfoW(
        &osvi,
        VER_MAJORVERSION | VER_MINORVERSION,
        conditionMask
    );
}
