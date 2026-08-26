#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <memory>
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <endpointvolume.h>
#include <wrl/client.h>
#include <commctrl.h>
#include "string"

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
    AudioControl(std::shared_ptr<WASAPIProcess> process);

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
    /// <param name="trayWindow"></param>
    void Draw(HWND trayWindow);

    void UpdateUI();

    void SetPosition(int x, int y);

    static void LayoutAudioControls(std::unordered_map<DWORD, std::unique_ptr<AudioControl>>& processTable)
    {
        constexpr int margin = 10;
        constexpr int spacing = 10;
        constexpr int width = 40;

        int x = margin;

        for (auto& [pid, control] : processTable)
        {
            control->SetPosition(x, margin);

            x += width + spacing;
        }
    }

    HWND GetAudioSlider() {
        return AudioLevelSlider;
    }

    HWND GetAudioMuteToggle() {
        return AudioMuteToggle;
    }

    private:
    HWND AudioLevelSlider{};
    HWND AudioMuteToggle{};
    HWND processIcon{};
    HWND processName{};

    std::shared_ptr<WASAPIProcess> pWASAPIProcess;
};
