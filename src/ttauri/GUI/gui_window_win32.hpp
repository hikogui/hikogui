// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../GFX/gfx_surface_vulkan.hpp"
#include "gui_window.hpp"
#include <unordered_map>

struct HWND__;
using HWND = HWND__ *;

namespace tt {

class gui_window_win32 final : public gui_window {
public:
    using super = gui_window;
    using delegate_type = typename super::delegate_type;

    HWND win32Window = nullptr;

    gui_window_win32(gui_system &gui, label const &title, std::weak_ptr<delegate_type> delegate = {}) noexcept;

    ~gui_window_win32();

    void create_window(extent2 new_size) override;
    int windowProc(unsigned int uMsg, uint64_t wParam, int64_t lParam) noexcept;

    void set_cursor(mouse_cursor cursor) noexcept override;

    void close_window() override;

    void minimize_window() override;

    void maximize_window() override;

    void normalize_window() override;

    void open_system_menu() override;

    void set_window_size(extent2 extent) override;

    [[nodiscard]] extent2 virtual_screen_size() const noexcept override;

    [[nodiscard]] std::string get_text_from_clipboard() const noexcept override;

    void set_text_on_clipboard(std::string str) noexcept override;

private:
    static constexpr UINT_PTR move_and_resize_timer_id = 2;

    TRACKMOUSEEVENT trackMouseLeaveEventParameters;
    bool trackingMouseLeaveEvent = false;
    char32_t highSurrogate = 0;
    mouse_event mouseButtonEvent;
    utc_nanoseconds doubleClickTimePoint;
    std::chrono::nanoseconds doubleClickMaximumDuration;

    void setOSWindowRectangleFromRECT(RECT aarectangle) noexcept;

    [[nodiscard]] KeyboardState getKeyboardState() noexcept;
    [[nodiscard]] keyboard_modifiers getkeyboard_modifiers() noexcept;

    [[nodiscard]] char32_t handleSuragates(char32_t c) noexcept;
    [[nodiscard]] mouse_event createmouse_event(unsigned int uMsg, uint64_t wParam, int64_t lParam) noexcept;
};

} // namespace tt
