// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "gfx_surface_vulkan.hpp"
#include <unordered_map>

namespace hi::inline v1 {

class gui_window_vulkan_macos final : public gfx_surface_vulkan {
public:
    // HWND win32Window = nullptr;

    gui_window_vulkan_macos(std::shared_ptr<WindowDelegate> const &delegate, Label &&title);
    ~gui_window_vulkan_macos();

    gui_window_vulkan_macos(const gui_window_vulkan_macos &) = delete;
    gui_window_vulkan_macos &operator=(const gui_window_vulkan_macos &) = delete;
    gui_window_vulkan_macos(gui_window_vulkan_macos &&) = delete;
    gui_window_vulkan_macos &operator=(gui_window_vulkan_macos &&) = delete;

    static void createWindowClass();

    static bool firstWindowHasBeenOpened;

    vk::SurfaceKHR getSurface() const override;

    void set_cursor(mouse_cursor cursor) noexcept override;

    void close_window() override;

    void minimize_window() override;

    void maximize_window() override;

    void normalize_window() override;

    void set_window_size(std::size_t width, std::size_t height) override {}

    [[nodiscard]] std::string get_text_from_clipboard() const noexcept override
    {
        return "<clipboard>";
    }

    void set_text_on_clipboard(std::string str) noexcept override {}

private:
    // void setOSWindowRectangleFromRECT(RECT aarectangle) noexcept;

    // TRACKMOUSEEVENT trackMouseLeaveEventParameters;
    bool trackingMouseLeaveEvent = false;

    // static LRESULT CALLBACK _WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    /*! The application class will function as a main-thread trampoline for this class
     * methods that start with `mainThread`.
     */
    friend hi::Application_macos;
};

} // namespace hi::inline v1
