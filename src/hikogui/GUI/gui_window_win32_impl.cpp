// Copyright Take Vos 2019-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "../utility/win32_headers.hpp"

#include "gui_window_win32.hpp"
#include "gui_system.hpp"
#include "keyboard_virtual_key.hpp"
#include "theme_book.hpp"
#include "../GFX/module.hpp"
#include "../widgets/module.hpp"
#include "../telemetry/module.hpp"
#include "../unicode/module.hpp"
#include "../utility/module.hpp"
#include "../settings/module.hpp"
#include "../crt/module.hpp"
#include "../algorithm/module.hpp"
#include "../loop/module.hpp"
#include "../macros.hpp"
#include <new>

import hikogui_utility;

namespace hi::inline v1 {

static const wchar_t *win32WindowClassName = nullptr;
static WNDCLASSW win32WindowClass = {};
static bool win32WindowClassIsRegistered = false;
static bool firstWindowHasBeenOpened = false;

/** The win32 window message handler.
 * This function should not take any locks as _WindowProc is called recursively.
 */
LRESULT CALLBACK _WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
{
    if (uMsg == WM_CREATE && lParam) {
        hilet createData = std::launder(std::bit_cast<CREATESTRUCT *>(lParam));

        SetLastError(0);
        auto r = SetWindowLongPtrW(hwnd, GWLP_USERDATA, std::bit_cast<LONG_PTR>(createData->lpCreateParams));
        if (r != 0 || GetLastError() != 0) {
            hi_log_fatal("Could not set GWLP_USERDATA on window. '{}'", get_last_error_message());
        }
    }

    // It is assumed that GWLP_USERDATA is zero when the window is created. Because messages to
    // this window are send before WM_CREATE and there is no way to figure out to which actual window
    // these messages belong.
    auto window_userdata = GetWindowLongPtrW(hwnd, GWLP_USERDATA);
    if (window_userdata == 0) {
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    auto& window = *std::launder(std::bit_cast<gui_window_win32 *>(window_userdata));
    hi_axiom(loop::main().on_thread());

    // WM_CLOSE and WM_DESTROY will re-enter and run the destructor for `window`.
    // We can no longer call virtual functions on the `window` object.
    if (uMsg == WM_CLOSE) {
        // Listeners can close the window by calling the destructor on `window`.
        window.closing();
        return 0;

    } else if (uMsg == WM_DESTROY) {
        // Remove the window now, before DefWindowProc, which could recursively
        // Reuse the window as it is being cleaned up.
        SetLastError(0);
        auto r = SetWindowLongPtrW(hwnd, GWLP_USERDATA, NULL);
        if (r == 0 || GetLastError() != 0) {
            hi_log_fatal("Could not set GWLP_USERDATA on window. '{}'", get_last_error_message());
        }

        // Also remove the win32Window from the window, so that we don't get double DestroyWindow().
        window.win32Window = nullptr;
        return 0;

    } else {
        if (auto result = window.windowProc(uMsg, wParam, lParam); result != -1) {
            return result;
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

static void createWindowClass()
{
    if (!win32WindowClassIsRegistered) {
        // Register the window class.
        win32WindowClassName = L"HikoGUI Window Class";

        std::memset(&win32WindowClass, 0, sizeof(WNDCLASSW));
        win32WindowClass.style = CS_DBLCLKS;
        win32WindowClass.lpfnWndProc = _WindowProc;
        win32WindowClass.hInstance = static_cast<HINSTANCE>(crt_application_instance);
        win32WindowClass.lpszClassName = win32WindowClassName;
        win32WindowClass.hCursor = nullptr;
        RegisterClassW(&win32WindowClass);
    }
    win32WindowClassIsRegistered = true;
}

void gui_window_win32::create_window(extent2 new_size)
{
    // This function should be called during init(), and therefor should not have a lock on the window.
    hi_assert(loop::main().on_thread());

    createWindowClass();

    auto u16title = to_wstring(to_string(_title.text));

    hi_log_info("Create window of size {} with title '{}'", new_size, _title);

    // Recommended to set the dpi-awareness before opening any window.
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    // We are opening a popup window with a caption bar to cause drop-shadow to appear around
    // the window.
    win32Window = CreateWindowExW(
        0, // Optional window styles.
        win32WindowClassName, // Window class
        u16title.data(), // Window text
        WS_OVERLAPPEDWINDOW, // Window style
        // Size and position
        500,
        500,
        round_cast<int>(new_size.width()),
        round_cast<int>(new_size.height()),

        NULL, // Parent window
        NULL, // Menu
        reinterpret_cast<HINSTANCE>(crt_application_instance), // Instance handle
        this);
    if (win32Window == nullptr) {
        hi_log_fatal("Could not open a win32 window: {}", get_last_error_message());
    }

    // Now we extend the drawable area over the titlebar and and border, excluding the drop shadow.
    // At least one value needs to be postive for the drop-shadow to be rendered.
    MARGINS m{0, 0, 0, 1};
    DwmExtendFrameIntoClientArea(win32Window, &m);

    // Force WM_NCCALCSIZE to be send to the window.
    SetWindowPos(win32Window, nullptr, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

    if (!firstWindowHasBeenOpened) {
        hilet win32_window_ = win32Window;
        switch (gui_window_size::normal) {
        case gui_window_size::normal:
            ShowWindow(win32_window_, SW_SHOWNORMAL);
            break;
        case gui_window_size::minimized:
            ShowWindow(win32_window_, SW_SHOWMINIMIZED);
            break;
        case gui_window_size::maximized:
            ShowWindow(win32_window_, SW_SHOWMAXIMIZED);
            break;
        default:
            hi_no_default();
        }
        firstWindowHasBeenOpened = true;
    }

    track_mouse_leave_event_parameters.cbSize = sizeof(track_mouse_leave_event_parameters);
    track_mouse_leave_event_parameters.dwFlags = TME_LEAVE;
    track_mouse_leave_event_parameters.hwndTrack = win32Window;
    track_mouse_leave_event_parameters.dwHoverTime = HOVER_DEFAULT;

    ShowWindow(win32Window, SW_SHOW);

    auto _dpi = GetDpiForWindow(win32Window);
    if (_dpi == 0) {
        throw gui_error("Could not retrieve dpi for window.");
    }
    dpi = narrow_cast<float>(_dpi);

    surface = gui.gfx->make_surface(crt_application_instance, win32Window);
}

gui_window_win32::gui_window_win32(gui_system& gui, std::unique_ptr<widget_intf> widget) noexcept :
    gui_window(gui, std::move(widget)), track_mouse_leave_event_parameters()
{
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    _widget->set_window(this);

    // Execute a constraint check to determine initial window size.
    theme = gui.theme_book->find(*gui.selected_theme, os_settings::theme_mode()).transform(dpi);

    _widget_constraints = _widget->update_constraints();
    hilet new_size = _widget_constraints.preferred;

    // Reset the keyboard target to not focus anything.
    update_keyboard_target({});

    // For changes in setting on the OS we should reconstrain/layout/redraw the window
    // For example when the language or theme changes.
    _setting_change_cbt = os_settings::subscribe(
        [this] {
            ++global_counter<"gui_window:os_setting:constrain">;
            this->process_event({gui_event_type::window_reconstrain});
        },
        callback_flags::main);

    // Subscribe on theme changes.
    _selected_theme_cbt = gui.selected_theme.subscribe(
        [this](auto...) {
            ++global_counter<"gui_window:selected_theme:constrain">;
            this->process_event({gui_event_type::window_reconstrain});
        },
        callback_flags::main);

    // Delegate has been called, layout of widgets has been calculated for the
    // minimum and maximum size of the window.
    create_window(new_size);
}

gui_window_win32::~gui_window_win32()
{
    try {
        if (win32Window != nullptr) {
            DestroyWindow(win32Window);
            hi_assert(win32Window == nullptr);
            // hi_log_fatal("win32Window was not destroyed before Window '{}' was destructed.", title);
        }

    } catch (std::exception const& e) {
        hi_log_fatal("Could not properly destruct gui_window_win32. '{}'", e.what());
    }
}

void gui_window_win32::close_window()
{
    hi_axiom(loop::main().on_thread());
    if (not PostMessageW(win32Window, WM_CLOSE, 0, 0)) {
        hi_log_error("Could not send WM_CLOSE to window {}: {}", _title, get_last_error_message());
    }
}

void gui_window_win32::set_size_state(gui_window_size state) noexcept
{
    hi_axiom(loop::main().on_thread());

    if (_size_state == state) {
        return;
    }

    if (_size_state == gui_window_size::normal) {
        _restore_rectangle = rectangle;
    } else if (_size_state == gui_window_size::minimized) {
        ShowWindow(win32Window, SW_RESTORE);
        _size_state = gui_window_size::normal;
    }

    if (state == gui_window_size::normal) {
        hilet left = round_cast<int>(_restore_rectangle.left());
        hilet top = round_cast<int>(_restore_rectangle.top());
        hilet width = round_cast<int>(_restore_rectangle.width());
        hilet height = round_cast<int>(_restore_rectangle.height());
        hilet inv_top = round_cast<int>(os_settings::primary_monitor_rectangle().height()) - top;
        SetWindowPos(win32Window, HWND_TOP, left, inv_top, width, height, 0);
        _size_state = gui_window_size::normal;

    } else if (state == gui_window_size::minimized) {
        ShowWindow(win32Window, SW_MINIMIZE);
        _size_state = gui_window_size::minimized;

    } else if (state == gui_window_size::maximized) {
        hilet workspace = workspace_rectangle();
        hilet max_size = _widget_constraints.maximum;

        // Try to resize the window while keeping the toolbar in the same location.
        hilet width = std::min(max_size.width(), workspace.width());
        hilet height = std::min(max_size.height(), workspace.height());
        hilet left = std::clamp(rectangle.left(), workspace.left(), workspace.right() - width);
        hilet top = std::clamp(rectangle.top(), workspace.bottom() + height, workspace.top());
        hilet inv_top = os_settings::primary_monitor_rectangle().height() - top;
        SetWindowPos(
            win32Window,
            HWND_TOP,
            round_cast<int>(left),
            round_cast<int>(inv_top),
            round_cast<int>(width),
            round_cast<int>(height),
            0);
        _size_state = gui_window_size::maximized;

    } else if (state == gui_window_size::fullscreen) {
        hilet fullscreen = fullscreen_rectangle();
        hilet max_size = _widget_constraints.maximum;
        if (fullscreen.width() > max_size.width() or fullscreen.height() > max_size.height()) {
            // Do not go full screen if the widget is unable to go that large.
            return;
        }

        hilet left = round_cast<int>(fullscreen.left());
        hilet top = round_cast<int>(fullscreen.top());
        hilet width = round_cast<int>(fullscreen.width());
        hilet height = round_cast<int>(fullscreen.height());
        hilet inv_top = round_cast<int>(os_settings::primary_monitor_rectangle().height()) - top;
        SetWindowPos(win32Window, HWND_TOP, left, inv_top, width, height, 0);
        _size_state = gui_window_size::fullscreen;
    }
}

[[nodiscard]] aarectangle gui_window_win32::workspace_rectangle() const noexcept
{
    hilet monitor = MonitorFromWindow(win32Window, MONITOR_DEFAULTTOPRIMARY);
    if (monitor == NULL) {
        hi_log_error("Could not get monitor for the window.");
        return {0, 0, 1920, 1080};
    }

    MONITORINFO info;
    info.cbSize = sizeof(MONITORINFO);
    if (not GetMonitorInfo(monitor, &info)) {
        hi_log_error("Could not get monitor info for the window.");
        return {0, 0, 1920, 1080};
    }

    hilet left = narrow_cast<float>(info.rcWork.left);
    hilet top = narrow_cast<float>(info.rcWork.top);
    hilet right = narrow_cast<float>(info.rcWork.right);
    hilet bottom = narrow_cast<float>(info.rcWork.bottom);
    hilet width = right - left;
    hilet height = bottom - top;
    hilet inv_bottom = os_settings::primary_monitor_rectangle().height() - bottom;
    return aarectangle{left, inv_bottom, width, height};
}

[[nodiscard]] aarectangle gui_window_win32::fullscreen_rectangle() const noexcept
{
    hilet monitor = MonitorFromWindow(win32Window, MONITOR_DEFAULTTOPRIMARY);
    if (monitor == NULL) {
        hi_log_error("Could not get monitor for the window.");
        return {0, 0, 1920, 1080};
    }

    MONITORINFO info;
    info.cbSize = sizeof(MONITORINFO);
    if (not GetMonitorInfo(monitor, &info)) {
        hi_log_error("Could not get monitor info for the window.");
        return {0, 0, 1920, 1080};
    }

    hilet left = narrow_cast<float>(info.rcMonitor.left);
    hilet top = narrow_cast<float>(info.rcMonitor.top);
    hilet right = narrow_cast<float>(info.rcMonitor.right);
    hilet bottom = narrow_cast<float>(info.rcMonitor.bottom);
    hilet width = right - left;
    hilet height = bottom - top;
    hilet inv_bottom = os_settings::primary_monitor_rectangle().height() - bottom;
    return aarectangle{left, inv_bottom, width, height};
}

[[nodiscard]] hi::subpixel_orientation gui_window_win32::subpixel_orientation() const noexcept
{
    // The table for viewing distance are:
    //
    // - Phone/Watch: 10 inch
    // - Tablet: 15 inch
    // - Notebook/Desktop: 20 inch
    //
    // Pixels Per Degree = PPD = 2 * viewing_distance * resolution * tan(0.5 degree)
    constexpr auto tan_half_degree = 0.00872686779075879f;
    constexpr auto viewing_distance = 20.0f;

    hilet ppd = 2 * viewing_distance * dpi * tan_half_degree;

    if (ppd > 55.0f) {
        // High resolution displays do not require subpixel-aliasing.
        return hi::subpixel_orientation::unknown;
    } else {
        // The win32 API does not have a per-monitor subpixel-orientation.
        return os_settings::subpixel_orientation();
    }
}

void gui_window_win32::open_system_menu()
{
    hi_axiom(loop::main().on_thread());

    // Position the system menu on the left side, below the system menu button.
    hilet left = rectangle.left();
    hilet top = rectangle.top() - 30.0f;

    // Convert to y-axis down coordinate system
    hilet inv_top = os_settings::primary_monitor_rectangle().height() - top;

    // Open the system menu window and wait.
    hilet system_menu = GetSystemMenu(win32Window, false);
    hilet cmd = TrackPopupMenu(system_menu, TPM_RETURNCMD, round_cast<int>(left), round_cast<int>(inv_top), 0, win32Window, NULL);
    if (cmd > 0) {
        SendMessage(win32Window, WM_SYSCOMMAND, narrow_cast<WPARAM>(cmd), LPARAM{0});
    }
}

void gui_window_win32::set_window_size(extent2 new_extent)
{
    hi_axiom(loop::main().on_thread());

    RECT original_rect;
    if (not GetWindowRect(win32Window, &original_rect)) {
        hi_log_error("Could not get the window's rectangle on the screen.");
    }

    hilet new_width = round_cast<int>(new_extent.width());
    hilet new_height = round_cast<int>(new_extent.height());
    hilet new_x =
        os_settings::left_to_right() ? narrow_cast<int>(original_rect.left) : narrow_cast<int>(original_rect.right - new_width);
    hilet new_y = narrow_cast<int>(original_rect.top);

    SetWindowPos(
        win32Window,
        HWND_NOTOPMOST,
        new_x,
        new_y,
        new_width,
        new_height,
        SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOREDRAW | SWP_DEFERERASE | SWP_NOCOPYBITS | SWP_FRAMECHANGED);
}

[[nodiscard]] std::optional<gstring> gui_window_win32::get_text_from_clipboard() const noexcept
{
    if (not OpenClipboard(win32Window)) {
        // Another application could have the clipboard locked.
        hi_log_info("Could not open win32 clipboard '{}'", get_last_error_message());
        return std::nullopt;
    }

    hilet defer_CloseClipboard = defer([] {
        CloseClipboard();
    });

    UINT format = 0;
    while ((format = EnumClipboardFormats(format)) != 0) {
        switch (format) {
        case CF_TEXT:
        case CF_OEMTEXT:
        case CF_UNICODETEXT:
            {
                hilet cb_data = GetClipboardData(CF_UNICODETEXT);
                if (cb_data == nullptr) {
                    hi_log_error("Could not get clipboard data: '{}'", get_last_error_message());
                    return std::nullopt;
                }

                auto const *const wstr_c = static_cast<wchar_t const *>(GlobalLock(cb_data));
                if (wstr_c == nullptr) {
                    hi_log_error("Could not lock clipboard data: '{}'", get_last_error_message());
                    return std::nullopt;
                }

                hilet defer_GlobalUnlock = defer([cb_data] {
                    if (not GlobalUnlock(cb_data) and GetLastError() != ERROR_SUCCESS) {
                        hi_log_error("Could not unlock clipboard data: '{}'", get_last_error_message());
                    }
                });

                auto r = to_gstring(hi::to_string(std::wstring_view(wstr_c)));
                hi_log_debug("get_text_from_clipboard '{}'", to_string(r));
                return {std::move(r)};
            }
            break;

        default:;
        }
    }

    if (GetLastError() != ERROR_SUCCESS) {
        hi_log_error("Could not enumerator clipboard formats: '{}'", get_last_error_message());
    }

    return std::nullopt;
}

void gui_window_win32::put_text_on_clipboard(gstring_view text) const noexcept
{
    if (not OpenClipboard(win32Window)) {
        // Another application could have the clipboard locked.
        hi_log_info("Could not open win32 clipboard '{}'", get_last_error_message());
        return;
    }

    hilet defer_CloseClipboard = defer([] {
        CloseClipboard();
    });

    if (not EmptyClipboard()) {
        hi_log_error("Could not empty win32 clipboard '{}'", get_last_error_message());
        return;
    }

    auto wtext = hi::to_wstring(unicode_normalize(to_u32string(text), unicode_normalize_config::NFC_CRLF_noctr()));

    auto wtext_handle = GlobalAlloc(GMEM_MOVEABLE, (wtext.size() + 1) * sizeof(wchar_t));
    if (wtext_handle == nullptr) {
        hi_log_error("Could not allocate clipboard data '{}'", get_last_error_message());
        return;
    }

    hilet defer_GlobalFree([&wtext_handle] {
        if (wtext_handle != nullptr) {
            GlobalFree(wtext_handle);
        }
    });

    {
        auto wtext_c = static_cast<wchar_t *>(GlobalLock(wtext_handle));
        if (wtext_c == nullptr) {
            hi_log_error("Could not lock string data '{}'", get_last_error_message());
            return;
        }

        hilet defer_GlobalUnlock = defer([wtext_handle] {
            if (not GlobalUnlock(wtext_handle) and GetLastError() != ERROR_SUCCESS) {
                hi_log_error("Could not unlock string data '{}'", get_last_error_message());
            }
        });

        std::memcpy(wtext_c, wtext.c_str(), (wtext.size() + 1) * sizeof(wchar_t));
    }

    if (SetClipboardData(CF_UNICODETEXT, wtext_handle) == nullptr) {
        hi_log_error("Could not set clipboard data '{}'", get_last_error_message());
        return;
    } else {
        // Data was transferred to clipboard.
        wtext_handle = nullptr;
    }
}

void gui_window_win32::setOSWindowRectangleFromRECT(RECT new_rectangle) noexcept
{
    hi_axiom(loop::main().on_thread());

    // Convert bottom to y-axis up coordinate system.
    hilet inv_bottom = os_settings::primary_monitor_rectangle().height() - new_rectangle.bottom;

    hilet new_screen_rectangle = aarectangle{
        narrow_cast<float>(new_rectangle.left),
        narrow_cast<float>(inv_bottom),
        narrow_cast<float>(new_rectangle.right - new_rectangle.left),
        narrow_cast<float>(new_rectangle.bottom - new_rectangle.top)};

    if (rectangle.size() != new_screen_rectangle.size()) {
        ++global_counter<"gui_window_win32:os-resize:relayout">;
        this->process_event({gui_event_type::window_relayout});
    }

    rectangle = new_screen_rectangle;
}

void gui_window_win32::set_cursor(mouse_cursor cursor) noexcept
{
    hi_axiom(loop::main().on_thread());

    if (current_mouse_cursor == cursor) {
        return;
    }
    current_mouse_cursor = cursor;

    if (cursor == mouse_cursor::None) {
        return;
    }

    static auto idcAppStarting = LoadCursorW(nullptr, IDC_APPSTARTING);
    static auto idcArrow = LoadCursorW(nullptr, IDC_ARROW);
    static auto idcHand = LoadCursorW(nullptr, IDC_HAND);
    static auto idcIBeam = LoadCursorW(nullptr, IDC_IBEAM);
    static auto idcNo = LoadCursorW(nullptr, IDC_NO);

    auto idc = idcNo;
    switch (cursor) {
    case mouse_cursor::None:
        idc = idcAppStarting;
        break;
    case mouse_cursor::Default:
        idc = idcArrow;
        break;
    case mouse_cursor::Button:
        idc = idcHand;
        break;
    case mouse_cursor::TextEdit:
        idc = idcIBeam;
        break;
    default:
        hi_no_default();
    }

    SetCursor(idc);
}

[[nodiscard]] keyboard_modifiers gui_window_win32::get_keyboard_modifiers() noexcept
{
    // Documentation of GetAsyncKeyState() says that the held key is in the most-significant-bit.
    // Make sure it is signed, so that we can do a less-than 0 check. It looks like this function
    // was designed to be used this way.
    static_assert(std::is_signed_v<decltype(GetAsyncKeyState(VK_SHIFT))>);

    auto r = keyboard_modifiers::none;

    if (GetAsyncKeyState(VK_SHIFT) < 0) {
        r |= keyboard_modifiers::shift;
    }
    if (GetAsyncKeyState(VK_CONTROL) < 0) {
        r |= keyboard_modifiers::control;
    }
    if (GetAsyncKeyState(VK_MENU) < 0) {
        r |= keyboard_modifiers::alt;
    }
    if (GetAsyncKeyState(VK_LWIN) < 0 or GetAsyncKeyState(VK_RWIN) < 0) {
        r |= keyboard_modifiers::super;
    }

    return r;
}

[[nodiscard]] keyboard_state gui_window_win32::get_keyboard_state() noexcept
{
    auto r = keyboard_state::idle;

    if (GetKeyState(VK_CAPITAL) != 0) {
        r |= keyboard_state::caps_lock;
    }
    if (GetKeyState(VK_NUMLOCK) != 0) {
        r |= keyboard_state::num_lock;
    }
    if (GetKeyState(VK_SCROLL) != 0) {
        r |= keyboard_state::scroll_lock;
    }
    return r;
}

/** The win32 window message handler.
 * This function should not take any long-term-locks as windowProc is called recursively.
 */
int gui_window_win32::windowProc(unsigned int uMsg, uint64_t wParam, int64_t lParam) noexcept
{
    using namespace std::chrono_literals;

    gui_event mouse_event;
    hilet current_time = std::chrono::utc_clock::now();

    switch (uMsg) {
    case WM_CLOSE:
        // WM_DESTROY is handled inside `_windowProc` since it has to deal with lifetime of `this`.
        break;

    case WM_DESTROY:
        // WM_DESTROY is handled inside `_windowProc` since it has to deal with lifetime of `this`.
        break;

    case WM_CREATE:
        {
            hilet createstruct_ptr = std::launder(std::bit_cast<CREATESTRUCT *>(lParam));
            RECT new_rectangle;
            new_rectangle.left = createstruct_ptr->x;
            new_rectangle.top = createstruct_ptr->y;
            new_rectangle.right = createstruct_ptr->x + createstruct_ptr->cx;
            new_rectangle.bottom = createstruct_ptr->y + createstruct_ptr->cy;
            setOSWindowRectangleFromRECT(new_rectangle);
        }
        break;

    case WM_ERASEBKGND:
        return 1;

    case WM_PAINT:
        {
            hilet height = [this]() {
                hi_axiom(loop::main().on_thread());
                return rectangle.height();
            }();

            PAINTSTRUCT ps;
            BeginPaint(win32Window, &ps);

            hilet update_rectangle = aarectangle{
                narrow_cast<float>(ps.rcPaint.left),
                narrow_cast<float>(height - ps.rcPaint.bottom),
                narrow_cast<float>(ps.rcPaint.right - ps.rcPaint.left),
                narrow_cast<float>(ps.rcPaint.bottom - ps.rcPaint.top)};

            {
                hi_axiom(loop::main().on_thread());
                this->process_event({gui_event_type::window_redraw, update_rectangle});
            }

            EndPaint(win32Window, &ps);
        }
        break;

    case WM_NCPAINT:
        hi_axiom(loop::main().on_thread());
        this->process_event({gui_event_type::window_redraw, aarectangle{rectangle.size()}});
        break;

    case WM_SIZE:
        // This is called when the operating system is changing the size of the window.
        // However we do not support maximizing by the OS.
        hi_axiom(loop::main().on_thread());
        switch (wParam) {
        case SIZE_MAXIMIZED:
            ShowWindow(win32Window, SW_RESTORE);
            set_size_state(gui_window_size::maximized);
            break;
        case SIZE_MINIMIZED:
            _size_state = gui_window_size::minimized;
            break;
        case SIZE_RESTORED:
            _size_state = gui_window_size::normal;
            break;
        default:
            break;
        }
        break;

    case WM_TIMER:
        if (last_forced_redraw + 16.7ms < current_time) {
            // During sizing the event loop is blocked.
            // Render at about 60fps.
            loop::main().resume_once();
            last_forced_redraw = current_time;
        }
        break;

    case WM_SIZING:
        {
            hilet& rect_ptr = *std::launder(std::bit_cast<RECT *>(lParam));
            if (rect_ptr.right < rect_ptr.left or rect_ptr.bottom < rect_ptr.top) {
                hi_log_error(
                    "Invalid RECT received on WM_SIZING: left={}, right={}, bottom={}, top={}",
                    rect_ptr.left,
                    rect_ptr.right,
                    rect_ptr.bottom,
                    rect_ptr.top);

            } else {
                setOSWindowRectangleFromRECT(rect_ptr);
            }
        }
        break;

    case WM_MOVING:
        {
            hilet& rect_ptr = *std::launder(std::bit_cast<RECT *>(lParam));
            if (rect_ptr.right < rect_ptr.left or rect_ptr.bottom < rect_ptr.top) {
                hi_log_error(
                    "Invalid RECT received on WM_MOVING: left={}, right={}, bottom={}, top={}",
                    rect_ptr.left,
                    rect_ptr.right,
                    rect_ptr.bottom,
                    rect_ptr.top);

            } else {
                setOSWindowRectangleFromRECT(rect_ptr);
            }
        }
        break;

    case WM_WINDOWPOSCHANGED:
        {
            hilet windowpos_ptr = std::launder(std::bit_cast<WINDOWPOS *>(lParam));
            RECT new_rectangle;
            new_rectangle.left = windowpos_ptr->x;
            new_rectangle.top = windowpos_ptr->y;
            new_rectangle.right = windowpos_ptr->x + windowpos_ptr->cx;
            new_rectangle.bottom = windowpos_ptr->y + windowpos_ptr->cy;
            setOSWindowRectangleFromRECT(new_rectangle);
        }
        break;

    case WM_ENTERSIZEMOVE:
        hi_axiom(loop::main().on_thread());
        if (SetTimer(win32Window, move_and_resize_timer_id, 16, NULL) != move_and_resize_timer_id) {
            hi_log_error("Could not set timer before move/resize. {}", get_last_error_message());
        }
        resizing = true;
        break;

    case WM_EXITSIZEMOVE:
        hi_axiom(loop::main().on_thread());
        if (not KillTimer(win32Window, move_and_resize_timer_id)) {
            hi_log_error("Could not kill timer after move/resize. {}", get_last_error_message());
        }
        resizing = false;
        // After a manual move of the window, it is clear that the window is in normal mode.
        _restore_rectangle = rectangle;
        _size_state = gui_window_size::normal;
        this->process_event({gui_event_type::window_redraw, aarectangle{rectangle.size()}});
        break;

    case WM_ACTIVATE:
        hi_axiom(loop::main().on_thread());
        switch (wParam) {
        case 1: // WA_ACTIVE
        case 2: // WA_CLICKACTIVE
            active = true;
            break;
        case 0: // WA_INACTIVE
            active = false;
            break;
        default:
            hi_log_error("Unknown WM_ACTIVE value.");
        }
        ++global_counter<"gui_window_win32:WM_ACTIVATE:constrain">;
        this->process_event({gui_event_type::window_reconstrain});
        break;

    case WM_GETMINMAXINFO:
        {
            hi_axiom(loop::main().on_thread());
            hilet minmaxinfo = std::launder(std::bit_cast<MINMAXINFO *>(lParam));
            minmaxinfo->ptMaxSize.x = round_cast<LONG>(_widget_constraints.maximum.width());
            minmaxinfo->ptMaxSize.y = round_cast<LONG>(_widget_constraints.maximum.height());
            minmaxinfo->ptMinTrackSize.x = round_cast<LONG>(_widget_constraints.minimum.width());
            minmaxinfo->ptMinTrackSize.y = round_cast<LONG>(_widget_constraints.minimum.height());
            minmaxinfo->ptMaxTrackSize.x = round_cast<LONG>(_widget_constraints.maximum.width());
            minmaxinfo->ptMaxTrackSize.y = round_cast<LONG>(_widget_constraints.maximum.height());
        }
        break;

    case WM_UNICHAR:
        if (auto c = char_cast<char32_t>(wParam); c == UNICODE_NOCHAR) {
            // Tell the 3rd party keyboard handler application that we support WM_UNICHAR.
            return 1;

        } else if (hilet gc = ucd_get_general_category(c); not is_C(gc) and not is_M(gc)) {
            // Only pass code-points that are non-control and non-mark.
            process_event(gui_event::keyboard_grapheme(grapheme{c}));
        }
        break;

    case WM_DEADCHAR:
        if (auto c = handle_suragates(char_cast<char32_t>(wParam))) {
            if (hilet gc = ucd_get_general_category(c); not is_C(gc) and not is_M(gc)) {
                // Only pass code-points that are non-control and non-mark.
                process_event(gui_event::keyboard_partial_grapheme(grapheme{c}));
            }
        }
        break;

    case WM_CHAR:
        if (auto c = handle_suragates(char_cast<char32_t>(wParam))) {
            if (hilet gc = ucd_get_general_category(c); not is_C(gc) and not is_M(gc)) {
                // Only pass code-points that are non-control and non-mark.
                process_event(gui_event::keyboard_grapheme(grapheme{c}));
            }
        }
        break;

    case WM_SYSCOMMAND:
        if (wParam == SC_KEYMENU) {
            keymenu_pressed = true;
            process_event(gui_event{gui_event_type::keyboard_down, keyboard_virtual_key::menu});
            return 0;
        }
        break;

    case WM_KEYDOWN:
    case WM_KEYUP:
        {
            hilet extended = (narrow_cast<uint32_t>(lParam) & 0x01000000) != 0;
            hilet key_code = narrow_cast<int>(wParam);
            hilet key_modifiers = get_keyboard_modifiers();
            auto virtual_key = to_keyboard_virtual_key(key_code, extended, key_modifiers);

            if (std::exchange(keymenu_pressed, false) and uMsg == WM_KEYDOWN and virtual_key == keyboard_virtual_key::space) {
                // On windows, Alt followed by Space opens the menu of the window, which is called the system menu.
                virtual_key = keyboard_virtual_key::sysmenu;
            }

            if (virtual_key != keyboard_virtual_key::nul) {
                hilet key_state = get_keyboard_state();
                hilet event_type = uMsg == WM_KEYDOWN ? gui_event_type::keyboard_down : gui_event_type::keyboard_up;
                process_event(gui_event{event_type, virtual_key, key_modifiers, key_state});
            }
        }
        break;

    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_XBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    case WM_XBUTTONUP:
    case WM_LBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
    case WM_XBUTTONDBLCLK:
    case WM_MOUSEWHEEL:
    case WM_MOUSEHWHEEL:
    case WM_MOUSEMOVE:
    case WM_MOUSELEAVE:
        keymenu_pressed = false;
        process_event(create_mouse_event(uMsg, wParam, lParam));
        break;

    case WM_NCCALCSIZE:
        if (wParam == TRUE) {
            // When wParam is TRUE, simply returning 0 without processing the NCCALCSIZE_PARAMS rectangles
            // will cause the client area to resize to the size of the window, including the window frame.
            // This will remove the window frame and caption items from your window, leaving only the client area displayed.
            //
            // Starting with Windows Vista, removing the standard frame by simply
            // returning 0 when the wParam is TRUE does not affect frames that are
            // extended into the client area using the DwmExtendFrameIntoClientArea function.
            // Only the standard frame will be removed.
            return 0;
        }

        break;

    case WM_NCHITTEST:
        {
            hi_axiom(loop::main().on_thread());

            hilet x = narrow_cast<float>(GET_X_LPARAM(lParam));
            hilet y = narrow_cast<float>(GET_Y_LPARAM(lParam));

            // Convert to y-axis up coordinate system.
            hilet inv_y = os_settings::primary_monitor_rectangle().height() - y;

            hilet hitbox_type = _widget->hitbox_test(screen_to_window() * point2{x, inv_y}).type;

            switch (hitbox_type) {
            case hitbox_type::bottom_resize_border:
                set_cursor(mouse_cursor::None);
                return HTBOTTOM;
            case hitbox_type::top_resize_border:
                set_cursor(mouse_cursor::None);
                return HTTOP;
            case hitbox_type::left_resize_border:
                set_cursor(mouse_cursor::None);
                return HTLEFT;
            case hitbox_type::right_resize_border:
                set_cursor(mouse_cursor::None);
                return HTRIGHT;
            case hitbox_type::bottom_left_resize_corner:
                set_cursor(mouse_cursor::None);
                return HTBOTTOMLEFT;
            case hitbox_type::bottom_right_resize_corner:
                set_cursor(mouse_cursor::None);
                return HTBOTTOMRIGHT;
            case hitbox_type::top_left_resize_corner:
                set_cursor(mouse_cursor::None);
                return HTTOPLEFT;
            case hitbox_type::top_right_resize_corner:
                set_cursor(mouse_cursor::None);
                return HTTOPRIGHT;
            case hitbox_type::application_icon:
                set_cursor(mouse_cursor::None);
                return HTSYSMENU;
            case hitbox_type::move_area:
                set_cursor(mouse_cursor::None);
                return HTCAPTION;
            case hitbox_type::text_edit:
                set_cursor(mouse_cursor::TextEdit);
                return HTCLIENT;
            case hitbox_type::button:
                set_cursor(mouse_cursor::Button);
                return HTCLIENT;
            case hitbox_type::scroll_bar:
                set_cursor(mouse_cursor::Default);
                return HTCLIENT;
            case hitbox_type::_default:
                set_cursor(mouse_cursor::Default);
                return HTCLIENT;
            case hitbox_type::outside:
                set_cursor(mouse_cursor::None);
                return HTCLIENT;
            default:
                hi_no_default();
            }
        }
        break;

    case WM_SETTINGCHANGE:
        hi_axiom(loop::main().on_thread());
        os_settings::gather();
        break;

    case WM_DPICHANGED:
        {
            hi_axiom(loop::main().on_thread());
            // x-axis dpi value.
            dpi = narrow_cast<float>(LOWORD(wParam));

            // Use the recommended rectangle to resize and reposition the window
            hilet new_rectangle = std::launder(reinterpret_cast<RECT *>(lParam));
            SetWindowPos(
                win32Window,
                NULL,
                new_rectangle->left,
                new_rectangle->top,
                new_rectangle->right - new_rectangle->left,
                new_rectangle->bottom - new_rectangle->top,
                SWP_NOZORDER | SWP_NOACTIVATE);
            ++global_counter<"gui_window_win32:WM_DPICHANGED:constrain">;
            this->process_event({gui_event_type::window_reconstrain});

            hi_log_info("DPI has changed to {}", dpi);
        }
        break;

    default:
        break;
    }

    // Let DefWindowProc() handle it.
    return -1;
}

[[nodiscard]] char32_t gui_window_win32::handle_suragates(char32_t c) noexcept
{
    hi_axiom(loop::main().on_thread());

    if (c >= 0xd800 && c <= 0xdbff) {
        high_surrogate = ((c - 0xd800) << 10) + 0x10000;
        return 0;

    } else if (c >= 0xdc00 && c <= 0xdfff) {
        c = high_surrogate ? high_surrogate | (c - 0xdc00) : 0xfffd;
    }
    high_surrogate = 0;
    return c;
}

[[nodiscard]] gui_event gui_window_win32::create_mouse_event(unsigned int uMsg, uint64_t wParam, int64_t lParam) noexcept
{
    hi_axiom(loop::main().on_thread());

    auto r = gui_event{gui_event_type::mouse_move};
    r.keyboard_modifiers = get_keyboard_modifiers();
    r.keyboard_state = get_keyboard_state();

    hilet x = narrow_cast<float>(GET_X_LPARAM(lParam));
    hilet y = narrow_cast<float>(GET_Y_LPARAM(lParam));

    // Convert to y-axis up coordinate system, y is in window-local.
    hilet inv_y = rectangle.height() - y;

    // On Window 7 up to and including Window10, the I-beam cursor hot-spot is 2 pixels to the left
    // of the vertical bar. But most applications do not fix this problem.
    r.mouse().position = point2{x, inv_y};
    r.mouse().wheel_delta = {};
    if (uMsg == WM_MOUSEWHEEL) {
        r.mouse().wheel_delta.y() = GET_WHEEL_DELTA_WPARAM(wParam) * 10.0f / WHEEL_DELTA;
    } else if (uMsg == WM_MOUSEHWHEEL) {
        r.mouse().wheel_delta.x() = GET_WHEEL_DELTA_WPARAM(wParam) * 10.0f / WHEEL_DELTA;
    }

    // Track which buttons are down, in case the application wants to track multiple buttons being pressed down.
    r.mouse().down.left_button = (GET_KEYSTATE_WPARAM(wParam) & MK_LBUTTON) > 0;
    r.mouse().down.middle_button = (GET_KEYSTATE_WPARAM(wParam) & MK_MBUTTON) > 0;
    r.mouse().down.right_button = (GET_KEYSTATE_WPARAM(wParam) & MK_RBUTTON) > 0;
    r.mouse().down.x1_button = (GET_KEYSTATE_WPARAM(wParam) & MK_XBUTTON1) > 0;
    r.mouse().down.x2_button = (GET_KEYSTATE_WPARAM(wParam) & MK_XBUTTON2) > 0;

    // Check which buttons caused the mouse event.
    switch (uMsg) {
    case WM_LBUTTONUP:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
        r.mouse().cause.left_button = true;
        break;
    case WM_RBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONDBLCLK:
        r.mouse().cause.right_button = true;
        break;
    case WM_MBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONDBLCLK:
        r.mouse().cause.middle_button = true;
        break;
    case WM_XBUTTONUP:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONDBLCLK:
        r.mouse().cause.x1_button = (GET_XBUTTON_WPARAM(wParam) & XBUTTON1) > 0;
        r.mouse().cause.x2_button = (GET_XBUTTON_WPARAM(wParam) & XBUTTON2) > 0;
        break;
    case WM_MOUSEMOVE:
        if (mouse_button_event == gui_event_type::mouse_down) {
            r.mouse().cause = mouse_button_event.mouse().cause;
        }
        break;
    case WM_MOUSEWHEEL:
    case WM_MOUSEHWHEEL:
    case WM_MOUSELEAVE:
        break;
    default:
        hi_no_default();
    }

    hilet a_button_is_pressed = r.mouse().down.left_button or r.mouse().down.middle_button or r.mouse().down.right_button or
        r.mouse().down.x1_button or r.mouse().down.x2_button;

    switch (uMsg) {
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    case WM_XBUTTONUP:
        r.set_type(gui_event_type::mouse_up);
        if (mouse_button_event) {
            r.mouse().down_position = mouse_button_event.mouse().down_position;
        }
        r.mouse().click_count = 0;

        if (!a_button_is_pressed) {
            ReleaseCapture();
        }
        break;

    case WM_LBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
    case WM_XBUTTONDBLCLK:
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_XBUTTONDOWN:
        {
            hilet within_double_click_time = r.time_point - multi_click_time_point < os_settings::double_click_interval();
            hilet double_click_distance = std::sqrt(narrow_cast<float>(squared_hypot(r.mouse().position - multi_click_position)));
            hilet within_double_click_distance = double_click_distance < os_settings::double_click_distance();

            multi_click_count = within_double_click_time and within_double_click_distance ? multi_click_count + 1 : 1;
            multi_click_time_point = r.time_point;
            multi_click_position = r.mouse().position;

            r.set_type(gui_event_type::mouse_down);
            r.mouse().down_position = r.mouse().position;
            r.mouse().click_count = multi_click_count;

            // Track draging past the window borders.
            hi_assert_not_null(win32Window);
            SetCapture(win32Window);
        }
        break;

    case WM_MOUSEWHEEL:
    case WM_MOUSEHWHEEL:
        r.set_type(gui_event_type::mouse_wheel);
        break;

    case WM_MOUSEMOVE:
        {
            // XXX Make sure the mouse is moved enough for this to cause a drag event.
            r.set_type(a_button_is_pressed ? gui_event_type::mouse_drag : gui_event_type::mouse_move);
            if (mouse_button_event) {
                r.mouse().down_position = mouse_button_event.mouse().down_position;
                r.mouse().click_count = mouse_button_event.mouse().click_count;
            }
        }
        break;

    case WM_MOUSELEAVE:
        r.set_type(gui_event_type::mouse_exit_window);
        if (mouse_button_event) {
            r.mouse().down_position = mouse_button_event.mouse().down_position;
        }
        r.mouse().click_count = 0;

        // After this event we need to ask win32 to track the mouse again.
        tracking_mouse_leave_event = false;

        // Force current_mouse_cursor to None so that the Window is in a fresh
        // state when the mouse reenters it.
        current_mouse_cursor = mouse_cursor::None;
        break;

    default:
        hi_no_default();
    }

    // Make sure we start tracking mouse events when the mouse has entered the window again.
    // So that once the mouse leaves the window we receive a WM_MOUSELEAVE event.
    if (not tracking_mouse_leave_event and uMsg != WM_MOUSELEAVE) {
        auto *track_mouse_leave_event_parameters_p = &track_mouse_leave_event_parameters;
        if (not TrackMouseEvent(track_mouse_leave_event_parameters_p)) {
            hi_log_error("Could not track leave event '{}'", get_last_error_message());
        }
        tracking_mouse_leave_event = true;
    }

    // Remember the last time a button was pressed or released, so that we can convert
    // a move into a drag event.
    if (r == gui_event_type::mouse_down or r == gui_event_type::mouse_up or r == gui_event_type::mouse_exit_window) {
        mouse_button_event = r;
    }

    return r;
}

} // namespace hi::inline v1
