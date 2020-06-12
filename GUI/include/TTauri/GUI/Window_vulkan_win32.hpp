// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/Window_vulkan.hpp"
#include <unordered_map>

namespace tt {
class Application_win32;
}

namespace tt {

class Window_vulkan_win32 final : public Window_vulkan {
public:
    void *win32Window = nullptr;

    Window_vulkan_win32(std::shared_ptr<WindowDelegate> const &delegate, Label const &title);
    ~Window_vulkan_win32();

    Window_vulkan_win32(const Window_vulkan_win32 &) = delete;
    Window_vulkan_win32 &operator=(const Window_vulkan_win32 &) = delete;
    Window_vulkan_win32(Window_vulkan_win32 &&) = delete;
    Window_vulkan_win32 &operator=(Window_vulkan_win32 &&) = delete;

    void closingWindow() override;
    void openingWindow() override;

    void createWindow(const std::string &title, vec extent);
    int windowProc(unsigned int uMsg, uint64_t wParam, int64_t lParam);

    vk::SurfaceKHR getSurface() const override;

    void setCursor(Cursor cursor) noexcept override;

    void closeWindow() override;

    void minimizeWindow() override;

    void maximizeWindow() override;

    void normalizeWindow() override;

    void setWindowSize(ivec extent) override;


    [[nodiscard]] std::string getTextFromClipboard() const noexcept override;

    void setTextOnClipboard(std::string str) noexcept override;

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