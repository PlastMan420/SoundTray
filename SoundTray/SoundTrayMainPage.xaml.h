#pragma once

#include "SoundTrayMainPage.g.h"

namespace winrt::SoundTray::implementation
{
    struct SoundTrayMainPage : SoundTrayMainPageT<SoundTrayMainPage>
    {
        SoundTrayMainPage()
        {
            // Xaml objects should not call InitializeComponent during construction.
            // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
        }

        int32_t MyProperty();
        void MyProperty(int32_t value);
    };
}

namespace winrt::SoundTray::factory_implementation
{
    struct SoundTrayMainPage : SoundTrayMainPageT<SoundTrayMainPage, implementation::SoundTrayMainPage>
    {
    };
}
