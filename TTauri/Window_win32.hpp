#pragma once

#include "Window.hpp"

#include <Windows.h>

#include <unordered_map>

namespace TTauri {

class Instance;

class Window_win32 : public Window {
public:
    HWND win32Window;

    Window_win32(Instance *instance, std::shared_ptr<Delegate> delegate, const std::string &title, int win32Show);
    ~Window_win32();

    vk::SurfaceKHR Window_win32::createWindow(Instance *instance, const std::string &title, int win32Show);
    LRESULT windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    static void createWindowClass();

    static const wchar_t *win32WindowClassName;
    static WNDCLASS win32WindowClass;
    static bool win32WindowClassIsRegistered;
    static std::unordered_map<std::uintptr_t, Window_win32 *> win32WindowMap;
    static bool firstWindowHasBeenOpened;
};

}
