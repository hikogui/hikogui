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

    void create_window(const std::u8string &title, f32x4 extent) override;
    int windowProc(unsigned int uMsg, uint64_t wParam, int64_t lParam) noexcept;

    vk::SurfaceKHR getSurface() const override;

    void set_cursor(Cursor cursor) noexcept override;

    void close_window() override;

    void minimize_window() override;

    void maximize_window() override;

    void normalize_window() override;

    void set_window_size(f32x4 extent) override;

    [[nodiscard]] f32x4 virtual_screen_size() const noexcept override;

    [[nodiscard]] std::string get_text_from_clipboard() const noexcept override;

    void set_text_on_clipboard(std::string str) noexcept override;

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
