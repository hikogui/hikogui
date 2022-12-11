// Copyright Take Vos 2019, 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../GFX/gfx_surface_vulkan.hpp"
#include "gui_window.hpp"
#include <unordered_map>

struct HWND__;
using HWND = HWND__ *;

namespace hi::inline v1 {

class gui_window_win32 final : public gui_window {
public:
    using super = gui_window;

    HWND win32Window = nullptr;

    gui_window_win32(gui_system& gui, label const& title) noexcept;

    ~gui_window_win32();

    void create_window(extent2i new_size) override;
    int windowProc(unsigned int uMsg, uint64_t wParam, int64_t lParam) noexcept;

    void set_cursor(mouse_cursor cursor) noexcept override;
    void close_window() override;
    void set_size_state(gui_window_size state) noexcept override;
    [[nodiscard]] aarectanglei workspace_rectangle() const noexcept override;
    [[nodiscard]] aarectanglei fullscreen_rectangle() const noexcept override;
    [[nodiscard]] hi::subpixel_orientation subpixel_orientation() const noexcept override;
    void open_system_menu() override;
    void set_window_size(extent2i extent) override;
    [[nodiscard]] std::optional<std::string> get_text_from_clipboard() const noexcept override;
    void put_text_on_clipboard(std::string_view str) const noexcept override;

private:
    static constexpr UINT_PTR move_and_resize_timer_id = 2;

    TRACKMOUSEEVENT track_mouse_leave_event_parameters;
    bool tracking_mouse_leave_event = false;
    char32_t high_surrogate = 0;
    gui_event mouse_button_event;
    utc_nanoseconds multi_click_time_point;
    point2i multi_click_position;
    uint8_t multi_click_count;

    bool keymenu_pressed = false;

    void setOSWindowRectangleFromRECT(RECT aarectangle) noexcept;

    [[nodiscard]] keyboard_state get_keyboard_state() noexcept;
    [[nodiscard]] keyboard_modifiers get_keyboard_modifiers() noexcept;

    [[nodiscard]] char32_t handle_suragates(char32_t c) noexcept;
    [[nodiscard]] gui_event create_mouse_event(unsigned int uMsg, uint64_t wParam, int64_t lParam) noexcept;

    friend LRESULT CALLBACK _WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept;
};

} // namespace hi::inline v1
