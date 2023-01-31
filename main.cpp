#include <iostream>
#include <d3d12.h>
#include <string>
#include <Windows.h>
#include <wchar.h>
import Render.Error;
namespace error = render::error;
HWND main_windows_hwnd;

LRESULT CALLBACK MainWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
int Run();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int nShowCmd) {
    WNDCLASSEX window_class{0};
    window_class.cbSize = sizeof(WNDCLASSEX);
    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = MainWindowProc;
    window_class.hInstance = hInstance;
    window_class.hIcon = LoadIcon(0, IDC_ARROW);
    window_class.hCursor = LoadCursor(0, IDC_ARROW);
    window_class.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
    window_class.lpszMenuName =nullptr;
    window_class.lpszClassName = "FantasyEngineDemo";
    if (!RegisterClassEx(&window_class)) {
        MessageBox(0, ("Failed Register Windows Class: " + error::GetLastErrorAsString()).c_str(), nullptr, 0);

        return 0;
    }
    main_windows_hwnd = CreateWindowEx(0, "FantasyEngineDemo", "FantasyEngine",
                                     WS_OVERLAPPEDWINDOW,
                                     CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                     nullptr, nullptr, hInstance, nullptr);
    if (!main_windows_hwnd) {
        MessageBox(nullptr, ("Create Window Failed: " + error::GetLastErrorAsString()).c_str(), nullptr, 0);
        return 1;
    }
    ShowWindow(main_windows_hwnd, nShowCmd);
    UpdateWindow(main_windows_hwnd);
    return Run();
}

LRESULT CALLBACK MainWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_LBUTTONDBLCLK:
            MessageBox(0, "Test", "Demo", MB_OK);
            return 0;
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) DestroyWindow(main_windows_hwnd);
            return 0;
        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);
    }
}

int Run() {
    MSG msg {0};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return static_cast<int>(msg.wParam);
}