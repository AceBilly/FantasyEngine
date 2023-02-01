//
// Created by yhtse on 2023/2/1.
//
module;
#include <Windows.h>
#include <string>
#include <exception>
#include <utility>
export module window;

import common;
import Render.Error;

namespace error = render::error;

template<typename Func>
concept WindowProcType = requires(Func func, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  {func(hWnd, msg, wParam, lParam)} -> std::same_as<LRESULT>;
};

struct WindowDescriptor {
    HWND window_handle;
    Rect<uint8_t> window_pos_and_size;
    std::string window_title;
    HINSTANCE instance;
    int show_cmd;
};

export
namespace fantasy::window {
    class Window {
    public:
        Window(HINSTANCE instance,const int& show_cmd, const std::string& title, WindowProcType auto callback) {
          m_window_descriptor.instance = instance;
          m_window_descriptor.window_title = title;
          m_window_descriptor.show_cmd = show_cmd;
          InitialWindow(callback);
        }
        Window(const Window&)=delete;
        Window(Window&&)=delete;
        Window& operator=(const Window&)=delete;
        Window& operator=(Window&&)=delete;
        ~Window()= default;
      public:
        void CreateWindowCustom() {
          if (!(m_window_descriptor.window_handle =  CreateWindowEx(0, "FantasyEngineDemo", "FantasyEngine",
                                                                   WS_OVERLAPPEDWINDOW,
                                                                   CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                                                   nullptr, nullptr, m_window_descriptor.instance, nullptr))) {
            MessageBox(nullptr, ("Create Window Failed: " + error::GetLastErrorAsString()).c_str(), nullptr, 0);
            std::terminate();
          }
          ShowWindow(m_window_descriptor.window_handle, m_window_descriptor.show_cmd);
          UpdateWindow(m_window_descriptor.window_handle);
        }
        int Run() {
          MSG msg {nullptr};
          while (msg.message != WM_QUIT) {
            if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
              TranslateMessage(&msg);
              DispatchMessage(&msg);
            }
          }
          return static_cast<int>(msg.wParam);
        }
      private:
        void InitialWindow(WindowProcType auto callback) {
          thread_local WNDCLASSEX window_class{0};
          window_class.cbSize = sizeof(WNDCLASSEX);
          window_class.style = CS_HREDRAW | CS_VREDRAW;
          window_class.lpfnWndProc = callback;
          window_class.hInstance = m_window_descriptor.instance;
          window_class.hIcon = LoadIcon(0, IDC_ARROW);
          window_class.hCursor = LoadCursor(0, IDC_ARROW);
          window_class.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
          window_class.lpszMenuName =nullptr;
          window_class.lpszClassName = "FantasyEngineDemo";
          if (!RegisterClassEx(&window_class)) {
            MessageBox(nullptr, ("Failed Register Windows Class: " + error::GetLastErrorAsString()).c_str(), nullptr, 0);
            std::terminate();
          }
        }
    private:
        WindowDescriptor m_window_descriptor;  // only support a window
    };
}  // namespace fantasy window end