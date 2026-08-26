#include "pch.h"
#include "AudioControl.h"

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
        pWASAPIProcess->sProcessInfo.hProcessName.c_str(),
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

inline bool AudioControl::IsMuted() const
{
    BOOL muted = FALSE;
    pWASAPIProcess->volumeControl->GetMute(&muted);
    return muted != FALSE;
}

void AudioControl::SetPosition(int x, int y, int width, int height)
{
    constexpr int padding = 8;
    constexpr int iconSize = 20;
    constexpr int labelHeight = 24;
    SetWindowPos(
        hAudioControlHWnd,
        nullptr,
        x, y,
        width, height,
        SWP_NOZORDER | SWP_NOACTIVATE
    );

    // Vertical slider on the left
    int sliderX = padding;
    int sliderY = padding + labelHeight;
    int sliderW = 30;
    // Ensure slider is at least 150 px tall for usability
    int sliderH = max(150, height - sliderY - padding - 30);

    SetWindowPos(
        hAudioLevelSlider,
        nullptr,
        sliderX,
        sliderY,
        sliderW,
        sliderH,
        SWP_NOZORDER | SWP_NOACTIVATE
    );

    // Process icon
    SetWindowPos(
        hProcessIcon,
        nullptr,
        padding + sliderW + 6,
        padding,
        iconSize,
        iconSize,
        SWP_NOZORDER | SWP_NOACTIVATE
    );

    // Process name, immediately to the right of the icon
    SetWindowPos(
        hProcessName,
        nullptr,
        padding + sliderW + 6 + iconSize + 6,
        padding,
        width - (padding + sliderW + 6 + iconSize + 6) - padding,
        labelHeight,
        SWP_NOZORDER | SWP_NOACTIVATE
    );

    // Mute button at bottom-right
    // Mute button below the slider (centered under slider)
    int muteW = 60;
    int muteH = 25;
    int muteY = sliderY + sliderH + 6; // small gap below slider

    // Center mute under slider, but clamp inside control bounds
    int muteX = sliderX + (sliderW / 2) - (muteW / 2);
    int controlLeft = x + padding;
    int controlRight = x + width - padding;
    if (muteX < controlLeft)
        muteX = controlLeft;
    if (muteX + muteW > controlRight)
        muteX = controlRight - muteW;

    // ensure mute button stays within control vertical bounds
    if (muteY + muteH + padding > y + height) {
        // place at bottom with padding if there's no space below slider
        muteY = y + height - muteH - padding;
    }

    SetWindowPos(
        hAudioMuteToggle,
        nullptr,
        muteX,
        muteY,
        muteW,
        muteH,
        SWP_NOZORDER | SWP_NOACTIVATE
    );
}
