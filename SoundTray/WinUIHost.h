// Minimal WinUI host header - requires WinAppSDK and C++/WinRT
#pragma once
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.h>

namespace winui {
    void InitializeWinUI();
    void ShowTrayWindow();
    void HideTrayWindow();
}
