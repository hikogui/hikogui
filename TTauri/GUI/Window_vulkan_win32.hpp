// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Window_vulkan.hpp"
#include <unordered_map>
#include <Windows.h>

namespace TTauri {
class Application_win32;
}

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
    void openingWindow() override;

    void Window_vulkan_win32::createWindow(const std::string &title, u32extent2 extent);
    LRESULT windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    static void createWindowClass();

    static const wchar_t *win32WindowClassName;
    static WNDCLASSW win32WindowClass;
    static bool win32WindowClassIsRegistered;
    static std::shared_ptr<std::unordered_map<HWND, Window_vulkan_win32 *>> win32WindowMap;
    static bool firstWindowHasBeenOpened;

    vk::SurfaceKHR getSurface() const override;

    void setCursor(Cursor cursor) override {
        if (cursor == currentCursor) {
            return;
        }
        currentCursor = cursor;

        switch (cursor) {
        case Cursor::None:
            SetCursor(LoadCursorW(nullptr, IDC_APPSTARTING));
            break;
        case Cursor::Default:
            SetCursor(LoadCursorW(nullptr, IDC_ARROW));
            break;
        case Cursor::Clickable:
            SetCursor(LoadCursorW(nullptr, IDC_HAND));
            break;
        default:
            SetCursor(LoadCursorW(nullptr, IDC_NO));
            break;
        }
    }

    void closeWindow() override;

    void minimizeWindow() override;

    void maximizeWindow() override;

    void normalizeWindow() override;

private:
    void mainThreadClosingWindow();
    void mainThreadOpeningWindow();

    void mainThreadCloseWindow();
    void mainThreadMinimizeWindow();
    void mainThreadMaximizeWindow();
    void mainThreadNormalizeWindow();

    void setOSWindowRectangleFromRECT(RECT rect);

    TRACKMOUSEEVENT trackMouseLeaveEventParameters;
    bool trackingMouseLeaveEvent = false;

    static LRESULT CALLBACK _WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    /*! The application class will function as a main-thread trampoline for this class
     * methods that start with `mainThread`.
     */
    friend TTauri::Application_win32;
};
}
