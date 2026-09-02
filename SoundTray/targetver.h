#pragma once
/// By default, Windows applications link against Version 5.82 of comctl32.dll, which does not contain some more modern APIs.
/// To fix this, you must force Windows to load Version 6.0 of the common controls library, which provides modern features 
/// like high-DPI scaling, modern button styles, and image lists.
/// Required for comctl32 6.0 features like LoadIconWithScaleDown.
/// Requires Windows XP or later.
#pragma comment(linker, "\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' \
version='6.0.0.0' \
processorArchitecture='*' \
publicKeyToken='6595b64144ccf1df' \
language='*'\"")
#include <WinSDKVer.h>

#define _WIN32_WINNT 0x0601 // Windows 7 or later

// // Including SDKDDKVer.h defines the highest available Windows platform.
// If you wish to build your application for a previous Windows platform, include WinSDKVer.h and
// set the _WIN32_WINNT macro to the platform you wish to support before including SDKDDKVer.h.
#include <SDKDDKVer.h>

