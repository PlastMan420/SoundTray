#include "pch.h"
#include "AudioControl.h"
#include <algorithm>

AudioControl::AudioControl()
{}

AudioControl::~AudioControl()
{
    DestroyWindow(AudioLevelSlider);
    DestroyWindow(AudioMuteToggle);

    DestroyIcon(pWASAPIProcess->sProcessInfo.processIcon);
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
        WS_CHILD | WS_VISIBLE | TBS_VERT | TBS_AUTOTICKS,
        0, 0,
        40, 150,
        trayWindow,
        reinterpret_cast<HMENU>(IDC_AUDIO_SLIDER),
        GetModuleHandleW(nullptr),
        nullptr
    );

    SendMessageW(AudioLevelSlider, TBM_SETRANGE, TRUE, MAKELONG(0, 100));

    //  sGWLP_USERDATA
    //    - 21
    //    Sets the user data associated with the window.This data is intended for use by the application that created the window.Its value is initially zero.

    /*
        LONG_PTR
        A LONG_PTR is a long type used for pointer precision.It is used when casting a pointer to a long type to perform pointer arithmetic.
    */
    SetWindowLongPtrW(
        AudioLevelSlider,
        GWLP_USERDATA,
        reinterpret_cast<LONG_PTR>(this)
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

    //GWLP_USERDATA
    //    - 21
    //    Sets the user data associated with the window.This data is intended for use by the application that created the window.Its value is initially zero.
    SetWindowLongPtrW(
        AudioMuteToggle,
        GWLP_USERDATA,
        reinterpret_cast<LONG_PTR>(this)
    );

    processIcon = CreateWindowExW(
        0,
        L"STATIC",
        nullptr,
        WS_CHILD | WS_VISIBLE | SS_ICON,
        0,
        190,
        20,
        20,
        trayWindow,
        nullptr,
        GetModuleHandleW(nullptr),
        nullptr
    );

    SendMessageW(
        processIcon,
        STM_SETICON,
        reinterpret_cast<WPARAM>(pWASAPIProcess->sProcessInfo.processIcon),
        0);

    processName = CreateWindowExW(
        0,
        L"STATIC",
        pWASAPIProcess->sProcessInfo.processName.c_str(),
        WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOPREFIX,
        25,
        190,
        100,
        20,
        trayWindow,
        nullptr,
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

void AudioControl::ToggleMuteState()
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

    SetWindowPos(
        processIcon,
        nullptr,
        x,
        y + 190,
        20,
        20,
        SWP_NOZORDER
    );

    SetWindowPos(
        processName,
        nullptr,
        x + 25,
        y + 190,
        100,
        20,
        SWP_NOZORDER
    );
}
