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
    int clickCount = 0;
    char32_t high_surrogate = 0;

    [[nodiscard]] KeyboardState getKeyboardState() noexcept;
    [[nodiscard]] KeyboardModifiers getKeyboardModifiers() noexcept;

    [[nodiscard]] char32_t handle_suragates(char32_t c) noexcept {
        if (c >= 0xd800 && c <= 0xdbff) {
            high_surrogate = ((c - 0xd800) << 10) + 0x10000;
            return 0;

        } else if (c >= 0xdc00 && c <= 0xdfff) {
            c = high_surrogate ? high_surrogate | (c - 0xdc00) : 0xfffd;
        }
        high_surrogate = 0;
        return c;
    }
};

}