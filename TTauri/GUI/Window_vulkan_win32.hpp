// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Window_vulkan.hpp"
#include <unordered_map>
#include <Windows.h>

namespace TTauri::GUI {

class Window_vulkan_win32 final : public Window_vulkan {
public:
    HWND win32Window = nullptr;

    Window_vulkan_win32(const std::shared_ptr<WindowDelegate> delegate, const std::string title);
    ~Window_vulkan_win32();

    Window_vulkan_win32(const Window_vulkan_win32 &) = delete;
    Window_vulkan_win32 &operator=(const Window_vulkan_win32 &) = delete;
    Window_vulkan_win32(Window_vulkan_win32 &&) = delete;
    Window_vulkan_win32 &operator=(Window_vulkan_win32 &&) = delete;

    void closingWindow() override;
    void mainThreadClosingWindow();
    void openingWindow() override;
    void mainThreadOpeningWindow();

    void Window_vulkan_win32::createWindow(const std::string &title);
    LRESULT windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    static void createWindowClass();

    static const wchar_t *win32WindowClassName;
    static WNDCLASSW win32WindowClass;
    static bool win32WindowClassIsRegistered;
    static std::shared_ptr<std::unordered_map<HWND, Window_vulkan_win32 *>> win32WindowMap;
    static bool firstWindowHasBeenOpened;

    vk::SurfaceKHR getSurface() override;
private:

    static LRESULT CALLBACK _WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
}
