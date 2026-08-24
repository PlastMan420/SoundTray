#include "pch.h"
#include "horizontal_stack.h"
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

void LayoutHorizontal(
    HWND parent,
    const std::vector<HWND>& controls,
    int x,
    int y,
    int width,
    int height,
    int spacing)
{
    for (HWND hwnd : controls)
    {
        SetWindowPos(
            hwnd,
            nullptr,
            x,
            y,
            width,
            height,
            SWP_NOZORDER
        );

        x += width + spacing;
    }
}
