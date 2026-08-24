#pragma once
#include "wtypes.h"
#include <memory>
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <endpointvolume.h>
#include <wrl/client.h>

#define IDC_AUDIO_SLIDER 1001
#define IDC_AUDIO_MUTE   1002

struct WASAPIAudioManager {
    Microsoft::WRL::ComPtr<IMMDeviceEnumerator> deviceEnumerator;
    Microsoft::WRL::ComPtr<IMMDevice> device;
    Microsoft::WRL::ComPtr<IAudioSessionManager2> sessionManager;
};

struct WASAPIProcess {
    DWORD processId;
    Microsoft::WRL::ComPtr<ISimpleAudioVolume> volumeControl;
};

class AudioControl {
    public:
    AudioControl();
    AudioControl(std::shared_ptr<WASAPIProcess> process);

    /// <summary>
    /// On mute button toggle
    /// </summary>
    void UpdateMuteState();

    /// <summary>
    /// Set process volume. Invoked via slider control or similar means
    /// </summary>
    /// <param name="volume"></param>
    void SetVolume(float volume);

    inline bool IsMuted() const;

    private:
    HWND AudioLevelSlider;
    HWND AudioMuteToggle;
    std::shared_ptr<WASAPIProcess> pWASAPIProcess;

    /// <summary>
    /// Draw control
    /// </summary>
    /// <param name="trayWindow"></param>
    void Draw(HWND trayWindow);
};
