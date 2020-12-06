// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "gui_window_vulkan.hpp"
#include <unordered_map>

struct HWND__;
using HWND = HWND__ *;

namespace tt {
class Application_win32;
}

namespace tt {

class gui_window_vulkan_win32 final : public gui_window_vulkan {
public:
    HWND win32Window = nullptr;

    gui_window_vulkan_win32(gui_system &system, std::weak_ptr<gui_window_delegate> const &delegate, label const &title);
    ~gui_window_vulkan_win32();

    gui_window_vulkan_win32(const gui_window_vulkan_win32 &) = delete;
    gui_window_vulkan_win32 &operator=(const gui_window_vulkan_win32 &) = delete;
    gui_window_vulkan_win32(gui_window_vulkan_win32 &&) = delete;
    gui_window_vulkan_win32 &operator=(gui_window_vulkan_win32 &&) = delete;

    void createWindow(const std::u8string &title, f32x4 extent) override;
    int windowProc(unsigned int uMsg, uint64_t wParam, int64_t lParam) noexcept;

    vk::SurfaceKHR getSurface() const override;

    void setCursor(Cursor cursor) noexcept override;

    void closeWindow() override;

    void minimizeWindow() override;

    void maximizeWindow() override;

    void normalizeWindow() override;

    void setWindowSize(ivec extent) override;

    [[nodiscard]] ivec virtualScreenSize() const noexcept override;

    [[nodiscard]] std::u8string getTextFromClipboard() const noexcept override;

    void setTextOnClipboard(std::u8string str) noexcept override;

private:
    void setOSWindowRectangleFromRECT(RECT aarect) noexcept;

    TRACKMOUSEEVENT trackMouseLeaveEventParameters;
    bool trackingMouseLeaveEvent = false;
    char32_t highSurrogate = 0;
    MouseEvent mouseButtonEvent;
    hires_utc_clock::time_point doubleClickTimePoint;
    hires_utc_clock::duration doubleClickMaximumDuration;

    [[nodiscard]] KeyboardState getKeyboardState() noexcept;
    [[nodiscard]] KeyboardModifiers getKeyboardModifiers() noexcept;

    [[nodiscard]] char32_t handleSuragates(char32_t c) noexcept;
    [[nodiscard]] MouseEvent createMouseEvent(unsigned int uMsg, uint64_t wParam, int64_t lParam) noexcept;


};

}
