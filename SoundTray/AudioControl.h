#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <memory>
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <wrl/client.h>
#include <commctrl.h>
#include <string>

#pragma comment(lib, "Comctl32.lib")

#define IDC_AUDIO_SLIDER 1001
#define IDC_AUDIO_MUTE   1002

struct WASAPIAudioManager {
    Microsoft::WRL::ComPtr<IMMDeviceEnumerator> deviceEnumerator;
    Microsoft::WRL::ComPtr<IMMDevice> device;
    Microsoft::WRL::ComPtr<IAudioSessionManager2> sessionManager;
};

struct ProcessInfo
{
    std::wstring processName;
    HICON processIcon{};
};

struct WASAPIProcess {
    DWORD processId;
    Microsoft::WRL::ComPtr<ISimpleAudioVolume> volumeControl;
    ProcessInfo sProcessInfo = {};
};

constexpr int CONTROL_WIDTH = 60;
constexpr int CONTROL_HEIGHT = 190;
constexpr int CONTROL_SPACING = 10;

class AudioControl {
    public:
    AudioControl();
    ~AudioControl();
    AudioControl(HINSTANCE program, HWND parent, std::shared_ptr<WASAPIProcess> process);

    inline static const std::wstring AudioControlWindowClassName = std::wstring(L"class__audiocontrolwindow");

    /// <summary>
    /// On mute button toggle
    /// </summary>
    void ToggleMuteState();

    /// <summary>
    /// Set process volume. Invoked via slider control or similar means
    /// </summary>
    /// <param name="volume"></param>
    void SetVolume(float volume);

    inline bool IsMuted() const;

    /// <summary>
    /// Draw control
    /// </summary>
    void Draw();

    void UpdateUI();

    void SetPosition(int x, int y, int width, int height);

    HWND GetAudioSlider() {
        return AudioLevelSlider;
    }

    HWND GetAudioMuteToggle() {
        return AudioMuteToggle;
    }

    static LRESULT CALLBACK AudioControlWndProc(
        HWND hwnd,
        UINT msg,
        WPARAM wParam,
        LPARAM lParam)
    {
        AudioControl* control =
            reinterpret_cast<AudioControl*>(
                GetWindowLongPtrW(hwnd, GWLP_USERDATA)
                );

        switch (msg)
        {
        case WM_NCCREATE:
        {
            auto* create =
                reinterpret_cast<CREATESTRUCTW*>(lParam);

            control =
                static_cast<AudioControl*>(create->lpCreateParams);

            SetWindowLongPtrW(
                hwnd,
                GWLP_USERDATA,
                reinterpret_cast<LONG_PTR>(control)
            );

            return TRUE;
        }

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            // Drawing...
            EndPaint(hwnd, &ps);

            return 0;
        }

        case WM_HSCROLL:
        case WM_VSCROLL:
        {
            // Trackbar sends WM_HSCROLL to parent even when vertical; handle both for safety.
            HWND slider = reinterpret_cast<HWND>(lParam);

            // If lParam is null (some scroll messages), try to derive slider from focus
            if (!slider)
                slider = reinterpret_cast<HWND>(GetDlgItem(hwnd, IDC_AUDIO_SLIDER));

            auto* control = reinterpret_cast<AudioControl*>(
                GetWindowLongPtrW(slider, GWLP_USERDATA)
                );

            if (control && slider)
            {
                int position = static_cast<int>(
                    SendMessageW(slider, TBM_GETPOS, 0, 0)
                    );

                // For vertical slider we map 0..100 to 1.0..0.0
                float volume = 1.0f - (position / 100.0f);
                control->SetVolume(volume);
            }

            return 0;
        }

        default:
            return DefWindowProcW(hwnd, msg, wParam, lParam);
        }
    }

    static ATOM RegisterAudioControlWindow() {
        WNDCLASSEXW wcex{};

        wcex.cbSize = sizeof(WNDCLASSEXW);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = AudioControl::AudioControlWndProc;
        wcex.hInstance = GetModuleHandleW(nullptr);
        wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        wcex.lpszClassName = AudioControlWindowClassName.data();

        return RegisterClassExW(&wcex);
    }

    private:
    HWND AudioLevelSlider{};
    HWND AudioMuteToggle{};
    HWND processIcon{};
    HWND processName{};
    HWND AudioControlHWnd{};

    std::shared_ptr<WASAPIProcess> pWASAPIProcess;

};
