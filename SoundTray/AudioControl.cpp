#include "pch.h"
#include "AudioControl.h"
#include <algorithm>

AudioControl::AudioControl()
{}

AudioControl::~AudioControl()
{
    DestroyWindow(AudioLevelSlider);
    DestroyWindow(AudioMuteToggle);
}

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

void AudioControl::UpdateUI()
{
    float volume = 0.0f;
    pWASAPIProcess->volumeControl->GetMasterVolume(&volume);

    SendMessageW(
        AudioLevelSlider,
        TBM_SETPOS,
        TRUE,
        static_cast<LPARAM>(volume * 100.0f)
    );

    BOOL muted = FALSE;
    pWASAPIProcess->volumeControl->GetMute(&muted);

    SendMessageW(
        AudioMuteToggle,
        BM_SETCHECK,
        muted ? BST_CHECKED : BST_UNCHECKED,
        0
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

void AudioControl::SetPosition(int x, int y)
{
    SetWindowPos(
        AudioLevelSlider,
        nullptr,
        x,
        y,
        40,
        150,
        SWP_NOZORDER
    );

    SetWindowPos(
        AudioMuteToggle,
        nullptr,
        x,
        y + 155,
        40,
        30,
        SWP_NOZORDER
    );
}
