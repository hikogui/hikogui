// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Window_vulkan.hpp"
#include <unordered_map>

namespace tt {
class Application_macos;
}

namespace tt {

class Window_vulkan_macos final : public Window_vulkan {
public:
    //HWND win32Window = nullptr;

    Window_vulkan_macos(std::shared_ptr<WindowDelegate> const &delegate, Label &&title);
    ~Window_vulkan_macos();

    Window_vulkan_macos(const Window_vulkan_macos &) = delete;
    Window_vulkan_macos &operator=(const Window_vulkan_macos &) = delete;
    Window_vulkan_macos(Window_vulkan_macos &&) = delete;
    Window_vulkan_macos &operator=(Window_vulkan_macos &&) = delete;

    void closingWindow() override;
    void openingWindow() override;

    void createWindow(const std::string &title, ivec extent);
    //LRESULT windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    static void createWindowClass();

    //static const wchar_t *win32WindowClassName;
    //static WNDCLASSW win32WindowClass;
    //static bool win32WindowClassIsRegistered;
    //static std::unordered_map<HWND, Window_vulkan_macos *> win32WindowMap;
    static bool firstWindowHasBeenOpened;

    vk::SurfaceKHR getSurface() const override;

    void setCursor(Cursor cursor) noexcept override;

    void closeWindow() override;

    void minimizeWindow() override;

    void maximizeWindow() override;

    void normalizeWindow() override;

    void setWindowSize(ivec extent) override {}

    [[nodiscard]] std::string getTextFromClipboard() const noexcept override {
        return "<clipboard>";
    }

    void setTextOnClipboard(std::string str) noexcept override { }


private:
    //void setOSWindowRectangleFromRECT(RECT aarect) noexcept;

    //TRACKMOUSEEVENT trackMouseLeaveEventParameters;
    bool trackingMouseLeaveEvent = false;

    //static LRESULT CALLBACK _WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    /*! The application class will function as a main-thread trampoline for this class
     * methods that start with `mainThread`.
     */
    friend tt::Application_macos;
};

}
