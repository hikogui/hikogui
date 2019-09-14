// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/Window_vulkan.hpp"
#include <unordered_map>

namespace TTauri {
class Application_win32;
}

namespace TTauri::GUI {

class Window_vulkan_win32 final : public Window_vulkan {
public:
    void *win32Window = nullptr;

    Window_vulkan_win32(const std::shared_ptr<WindowDelegate> delegate, const std::string title);
    ~Window_vulkan_win32();

    Window_vulkan_win32(const Window_vulkan_win32 &) = delete;
    Window_vulkan_win32 &operator=(const Window_vulkan_win32 &) = delete;
    Window_vulkan_win32(Window_vulkan_win32 &&) = delete;
    Window_vulkan_win32 &operator=(Window_vulkan_win32 &&) = delete;

    void closingWindow() override;
    void openingWindow() override;

    void createWindow(const std::string &title, extent2 extent);
    int windowProc(unsigned int uMsg, uint64_t wParam, int64_t lParam);

    vk::SurfaceKHR getSurface() const override;

    void setCursor(Cursor cursor) noexcept override;

    void closeWindow() override;

    void minimizeWindow() override;

    void maximizeWindow() override;

    void normalizeWindow() override;

private:
    void setOSWindowRectangleFromRECT(RECT rect) noexcept;

    TRACKMOUSEEVENT trackMouseLeaveEventParameters;
    bool trackingMouseLeaveEvent = false;
};

}