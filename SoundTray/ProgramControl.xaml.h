#pragma once

#include "ProgramControl.g.h"

namespace winrt::SoundTray::implementation
{
    struct ProgramControl : ProgramControlT<ProgramControl>
    {
        ProgramControl()
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
    struct ProgramControl : ProgramControlT<ProgramControl, implementation::ProgramControl>
    {
    };
}
