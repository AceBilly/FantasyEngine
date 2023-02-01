#include <iostream>
#include <d3d12.h>
#include <string>
#include <Windows.h>
#include <wchar.h>
import Render.Error;
import window;
namespace error = render::error;
using namespace fantasy::window;
using namespace std::literals::string_literals;


LRESULT CALLBACK MainWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int nShowCmd) {
  Window window(hInstance, nShowCmd, "FantasyEngine"s, MainWindowProc);
  window.CreateWindowCustom();
  return window.Run();
}

LRESULT CALLBACK MainWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_LBUTTONDBLCLK:
            MessageBox(0, "Test", "Demo", MB_OK);
            return 0;
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) DestroyWindow(hWnd);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);
    }
}

