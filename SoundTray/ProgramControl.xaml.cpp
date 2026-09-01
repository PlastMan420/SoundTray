#include "pch.h"
#include "ProgramControl.xaml.h"
#if __has_include("ProgramControl.g.cpp")
#include "ProgramControl.g.cpp"
#endif

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::SoundTray::implementation
{
    int32_t ProgramControl::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void ProgramControl::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }
}
