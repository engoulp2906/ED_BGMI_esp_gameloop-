#include "Overlay.h"
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

Overlay::Overlay(const wchar_t* windowTitle, HWND targetWindow)
    : m_hWnd(NULL), m_targetWindow(targetWindow), m_hInstance(GetModuleHandle(NULL)), m_width(0), m_height(0) {
}

Overlay::~Overlay() {
    if (m_hWnd) DestroyWindow(m_hWnd);
}

bool Overlay::CreateOverlayWindow(int width, int height) {
    m_width = width;
    m_height = height;

    WNDCLASS wc = { };
    wc.lpfnWndProc = Overlay::WindowProc;
    wc.hInstance = m_hInstance;
    wc.lpszClassName = L"ESPOverlayClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClass(&wc)) {
        return false;
    }

    m_hWnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT,
        wc.lpszClassName,
        L"ESP Overlay",
        WS_POPUP,
        0, 0, m_width, m_height,
        NULL,
        NULL,
        m_hInstance,
        NULL);

    if (!m_hWnd) return false;

    // Make the window transparent and click-through
    SetLayeredWindowAttributes(m_hWnd, 0, 255, LWA_ALPHA);
    MARGINS margins = { -1 };
    DwmExtendFrameIntoClientArea(m_hWnd, &margins);

    ShowWindow(m_hWnd, SW_SHOW);
    UpdateWindow(m_hWnd);

    return true;
}

void Overlay::RenderDot(int x, int y) {
    if (!m_hWnd) return;

    HDC hdc = GetDC(m_hWnd);
    if (!hdc) return;

    // Draw a small red dot (filled ellipse)
    HBRUSH brush = CreateSolidBrush(RGB(255, 0, 0));
    Ellipse(hdc, x - 3, y - 3, x + 3, y + 3);
    DeleteObject(brush);

    ReleaseDC(m_hWnd, hdc);
}

void Overlay::RunMessageLoop() {
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

LRESULT CALLBACK Overlay::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}