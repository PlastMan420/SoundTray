#include "pch.h"
#include "SoundTrayMainPage.xaml.h"
#if __has_include("SoundTrayMainPage.g.cpp")
#include "SoundTrayMainPage.g.cpp"
#endif

namespace winrt::SoundTray::implementation
{
    int32_t SoundTrayMainPage::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void SoundTrayMainPage::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }
}
