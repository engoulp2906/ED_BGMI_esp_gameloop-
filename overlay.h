#pragma once
#include <windows.h>

class Overlay {
public:
    Overlay(const wchar_t* windowTitle, HWND targetWindow);
    ~Overlay();

    bool CreateOverlayWindow(int width, int height);
    void RenderDot(int x, int y);
    void RunMessageLoop();

private:
    HWND m_hWnd;
    HWND m_targetWindow;
    HINSTANCE m_hInstance;
    int m_width;
    int m_height;

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
}; 
