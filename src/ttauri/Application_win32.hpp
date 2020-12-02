// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "application.hpp"
#include <thread>

namespace tt {

// WM_USER = ?-0x7fff
// WM_APP = 0x8000-0xbfff.
constexpr unsigned int WM_WIN_LANGUAGE_CHANGE = 0x8000 - 1;
constexpr unsigned int WM_APP_CALL_FUNCTION = 0x8000 + 1;

class application_win32 final : public application {
public:
    uint32_t OSMainThreadID = 0;

    /** Windows GUI-application instance handle.
     */
    void *hInstance = nullptr;

    /** Windows GUI-application startup command.
     */
    int nCmdShow = 0;

    application_win32(std::weak_ptr<application_delegate> const &delegate, void *hInstance, int nCmdShow);
    ~application_win32() = default;

    application_win32(const application_win32 &) = delete;
    application_win32 &operator=(const application_win32 &) = delete;
    application_win32(application_win32 &&) = delete;
    application_win32 &operator=(application_win32 &&) = delete;

    void init() override;

    void run_from_main_loop(std::function<void()> function) override;

    int loop() override;

    void exit(int exit_code) override;

    /** Get a win32 handle to each window of the application.
     */
    [[nodiscard]] std::vector<void *> win32_windows() noexcept;

    /** Send a win32 message to a window, on the thread associated with that window.
     */
    void post_message(void *window, unsigned int Msg, ptrdiff_t wParam = 0, ptrdiff_t lParam = 0) noexcept;

    /** Send a win32 message to a list of windows, on the thread associated with each of those windows.
     */
    void post_message(std::vector<void *> const &windows, unsigned int Msg, ptrdiff_t wParam = 0, ptrdiff_t lParam = 0) noexcept;

protected:
    typename timer::callback_ptr_type languages_maintenance_callback;

    void audioStart() override;
};

} // namespace tt
