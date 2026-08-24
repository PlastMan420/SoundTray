#include "pch.h"
#include "AudioControl.h"
#include <algorithm>

AudioControl::AudioControl()
{}

AudioControl::AudioControl(std::shared_ptr<WASAPIProcess> process)
{
    pWASAPIProcess = process;
}

void AudioControl::SetVolume(float volume)
{
    volume = std::clamp(volume, 0.0f, 1.0f);

    pWASAPIProcess->volumeControl->SetMasterVolume(volume, nullptr);
}

void AudioControl::Draw(HWND trayWindow)
{
    AudioLevelSlider = CreateWindowExW(
        0,
        TRACKBAR_CLASSW,
        nullptr,
        WS_CHILD | WS_VISIBLE |
        TBS_VERT |
        TBS_AUTOTICKS,
        0, 0,
        40, 150,
        trayWindow,
        reinterpret_cast<HMENU>(IDC_AUDIO_SLIDER),
        GetModuleHandleW(nullptr),
        nullptr
    );

    // level = 0.0f - 1.0f
    float level = 0.0f;
    pWASAPIProcess->volumeControl->GetMasterVolume(&level);

    SendMessageW(AudioLevelSlider, TBM_SETRANGE, TRUE, MAKELONG(0, 100));
    SendMessageW(AudioLevelSlider, TBM_SETPOS, TRUE, static_cast<int>(std::clamp(level, 0.0f, 1.0f) * 100.0f));

    AudioMuteToggle = CreateWindowExW(
        0,
        L"BUTTON",
        L"Mute",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        0, 155,
        60, 30,
        trayWindow,
        reinterpret_cast<HMENU>(IDC_AUDIO_MUTE),
        GetModuleHandleW(nullptr),
        nullptr
    );
}

void AudioControl::UpdateMuteState()
{
    BOOL muted = IsMuted();

    // Toggle mute.
    SendMessageW(
        AudioMuteToggle,
        BM_SETCHECK,
        muted ? BST_CHECKED : BST_UNCHECKED,
        0
    );
}

inline bool AudioControl::IsMuted() const
{
    BOOL muted = FALSE;
    pWASAPIProcess->volumeControl->GetMute(&muted);
    return muted != FALSE;
}
