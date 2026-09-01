#include "pch.h"
#include "WinUIHost.h"

// This file requires WinAppSDK (Microsoft.UI.Xaml) and C++/WinRT. It provides a minimal host
// that creates a Xaml Window and an ItemsControl. The app must link against the WinAppSDK.

#ifdef __has_include
#if __has_include(<winrt/Microsoft.UI.Xaml.h>)
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Controls;

static Window m_window{ nullptr };
static Controls::WrapGrid m_wrapGrid{ nullptr };
static Controls::ScrollViewer m_scroll{ nullptr };

void winui::InitializeWinUI()
{
    // Initialize C++/WinRT in STA apartment for UI
    winrt::init_apartment(winrt::apartment_type::single_threaded);
}

void EnsureWindow()
{
    if (m_window)
        return;

    m_window = Window();
    m_window.Activate();
    Grid grid;

    // Create a ScrollViewer containing a WrapGrid for tiled layout (max 4 columns)
    m_wrapGrid = Controls::WrapGrid();
    m_wrapGrid.Orientation(winrt::Microsoft::UI::Xaml::Controls::Orientation::Horizontal);
    m_wrapGrid.MaximumRowsOrColumns(4);
    m_wrapGrid.HorizontalAlignment(winrt::Microsoft::UI::Xaml::HorizontalAlignment::Left);

    m_scroll = Controls::ScrollViewer();
    m_scroll.Content(m_wrapGrid);
    grid.Children().Append(m_scroll);

    m_window.Content(grid);
}

void winui::ShowTrayWindow()
{
    EnsureWindow();
    // Populate items from ProcessManager::EnumerateAudioSessions
    try {
        m_wrapGrid.Children().Clear();
        auto list = ProcessManager::EnumerateAudioSessions(globals::sAudioDevices);
        for (const auto& p : list) {
            // Create a vertical stack: icon (skipped), slider, mute checkbox, text
            Controls::StackPanel panel;
            panel.Orientation(winrt::Microsoft::UI::Xaml::Controls::Orientation::Vertical);
            panel.Width(140);
            panel.Height(220);

            // Slider
            Controls::Slider slider;
            slider.Orientation(winrt::Microsoft::UI::Xaml::Controls::Orientation::Vertical);
            slider.Minimum(0);
            slider.Maximum(100);
            slider.Value( static_cast<double>(100) );
            slider.Height(150);
            panel.Children().Append(slider);

            // Mute checkbox
            Controls::CheckBox cb;
            cb.Content(winrt::box_value(L"Mute"));
            panel.Children().Append(cb);

            // Process text
            Controls::TextBlock tb;
            tb.Text(winrt::hstring(p->sProcessInfo.hProcessName));
            tb.TextWrapping(winrt::Microsoft::UI::Xaml::TextWrapping::NoWrap);
            panel.Children().Append(tb);

            m_wrapGrid.Children().Append(panel);
        }
    }
    catch (...) {
        OutputDebugStringW(L"winui::ShowTrayWindow: failed to populate items\n");
    }

    m_window.Activate();
}

void winui::HideTrayWindow()
{
    if (m_window)
        m_window.Close();
}

#else
void winui::InitializeWinUI() {}
void winui::ShowTrayWindow() {}
void winui::HideTrayWindow() {}
#endif
#endif
