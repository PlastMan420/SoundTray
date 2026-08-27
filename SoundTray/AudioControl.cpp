#include "pch.h"
#include "AudioControl.h"
#include "Resource.h"

AudioControl::AudioControl()
{}

AudioControl::~AudioControl()
{
    DestroyWindow(hAudioLevelSlider);
    DestroyWindow(hAudioMuteToggle);

    if (pWASAPIProcess && pWASAPIProcess->sProcessInfo.hProcessIcon)
        DestroyIcon(pWASAPIProcess->sProcessInfo.hProcessIcon);

    if (hAudioControlHWnd)
        DestroyWindow(hAudioControlHWnd);
}

AudioControl::AudioControl(HINSTANCE program, HWND parent, std::shared_ptr<WASAPIProcess> process)
{
    pWASAPIProcess = process;
    hAudioControlHWnd = CreateWindowExW(
        0,
        AudioControlWindowClassName.data(),
        nullptr,
        WS_CHILD | WS_VISIBLE,
        0, 0,
        100, 240,
        parent,
        nullptr,
        program,
        this
    );
}

void AudioControl::SetVolume(float volume)
{
    volume = max(0.0f, min(1.0f, volume));

    pWASAPIProcess->volumeControl->SetMasterVolume(volume, nullptr);
}

void AudioControl::Draw()
{
    RECT rc{};
    GetClientRect(hAudioControlHWnd, &rc);

    // Create a vertical trackbar. Initial size is small; SetPosition will reposition it.
    hAudioLevelSlider = CreateWindowExW(
        0,
        TRACKBAR_CLASSW,
        nullptr,
        WS_CHILD | WS_VISIBLE | TBS_VERT | TBS_AUTOTICKS | TBS_NOTICKS,
        0,
        0,
        30,
        150,
        hAudioControlHWnd,
        reinterpret_cast<HMENU>(IDC_AUDIO_SLIDER),
        GetModuleHandleW(nullptr),
        nullptr
    );

    SendMessageW(hAudioLevelSlider, TBM_SETRANGE, TRUE, MAKELONG(0, 100));
    SendMessageW(hAudioLevelSlider, TBM_SETPOS, TRUE, 0);

    SetWindowLongPtrW(
        hAudioLevelSlider,
        GWLP_USERDATA,
        reinterpret_cast<LONG_PTR>(this)
    );

    hAudioMuteToggle = CreateWindowExW(
        0,
        L"BUTTON",
        L"Mute",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        0, 155,
        60, 30,
        hAudioControlHWnd,
        reinterpret_cast<HMENU>(IDC_AUDIO_MUTE),
        GetModuleHandleW(nullptr),
        nullptr
    );

    SetWindowLongPtrW(
        hAudioMuteToggle,
        GWLP_USERDATA,
        reinterpret_cast<LONG_PTR>(this)
    );

    hProcessIcon = CreateWindowExW(
        0,
        L"STATIC",
        nullptr,
        WS_CHILD | WS_VISIBLE | SS_ICON,
        0,
        190,
        20,
        20,
        hAudioControlHWnd,
        nullptr,
        GetModuleHandleW(nullptr),
        nullptr
    );

    SendMessageW(
        hProcessIcon,
        STM_SETICON,
        reinterpret_cast<WPARAM>(pWASAPIProcess->sProcessInfo.hProcessIcon),
        0);

    hProcessName = CreateWindowExW(
        0,
        L"STATIC",
        pWASAPIProcess->sProcessInfo.hProcessName.data(),
        WS_CHILD | WS_VISIBLE | SS_LEFT | SS_ENDELLIPSIS,
        25,
        190,
        100,
        20,
        hAudioControlHWnd,
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
        hAudioLevelSlider,
        TBM_SETPOS,
        TRUE,
        static_cast<LPARAM>(volume * 100.0f)
    );

    BOOL muted = FALSE;
    pWASAPIProcess->volumeControl->GetMute(&muted);

    SendMessageW(
        hAudioMuteToggle,
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
        hAudioMuteToggle,
        BM_SETCHECK,
        muted ? BST_CHECKED : BST_UNCHECKED,
        0
    );
}


 bool AudioControl::IsMuted()
{
    BOOL muted = FALSE;
    pWASAPIProcess->volumeControl->GetMute(&muted);
    return muted != FALSE;
}

 void AudioControl::Mute()
{
    pWASAPIProcess->volumeControl->SetMute(true, NULL);

    SendMessageW(
        hAudioMuteToggle,
        BM_SETCHECK,
        BST_CHECKED,
        0
    );
}

 void AudioControl::UnMute()
{
    pWASAPIProcess->volumeControl->SetMute(FALSE, NULL);

    SendMessageW(
        hAudioMuteToggle,
        BM_SETCHECK,
        BST_UNCHECKED,
        0
    );
}

void AudioControl::SetPosition(
    int x,
    int y,
    int width,
    int height)
{
    constexpr int iconSize = 20;
    constexpr int sliderW = 30;
    constexpr int sliderH = 150;

    constexpr int muteW = 60;
    constexpr int muteH = 25;

    constexpr int spacing = 8;

    SetWindowPos(
        hAudioControlHWnd,
        nullptr,
        x,
        y,
        width,
        height,
        SWP_NOZORDER | SWP_NOACTIVATE
    );

    // Total height of:
    //
    // icon
    // gap
    // slider
    // gap
    // mute button
    //
    constexpr int contentHeight =
        iconSize +
        spacing +
        sliderH +
        spacing +
        muteH;

    // Center the entire group vertically.
    const int startY =
        max(0, (height - contentHeight) / 2);

    const int centerX = width / 2;

    // ---------------------------------------------------------
    // Process icon
    // ---------------------------------------------------------

    const int iconX =
        centerX - iconSize / 2;

    const int iconY =
        startY;

    SetWindowPos(
        hProcessIcon,
        nullptr,
        iconX,
        iconY,
        iconSize,
        iconSize,
        SWP_NOZORDER | SWP_NOACTIVATE
    );

    // ---------------------------------------------------------
    // Slider
    // ---------------------------------------------------------

    const int sliderX =
        centerX - sliderW / 2;

    const int sliderY =
        iconY + iconSize + spacing;

    SetWindowPos(
        hAudioLevelSlider,
        nullptr,
        sliderX,
        sliderY,
        sliderW,
        sliderH,
        SWP_NOZORDER | SWP_NOACTIVATE
    );

    // ---------------------------------------------------------
    // Mute button
    // ---------------------------------------------------------
    const int muteSize = 24;
    const int muteX = centerX - muteSize / 2;
    const int muteY = sliderY + sliderH + spacing;

    SetWindowPos(
        hAudioMuteToggle,
        nullptr,
        muteX,
        muteY,
        muteSize,
        muteSize,
        SWP_NOZORDER | SWP_NOACTIVATE
    );

    // No process name.
    ShowWindow(hProcessName, SW_HIDE);

    // No text on mute button.
    SetWindowTextW(
        hAudioMuteToggle,
        L""
    );
}