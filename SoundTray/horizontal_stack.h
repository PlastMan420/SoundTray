#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>

void LayoutHorizontal(
    HWND parent,
    const std::vector<HWND>& controls,
    int x,
    int y,
    int width,
    int height,
    int spacing);
