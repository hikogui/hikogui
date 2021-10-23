// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gui_window_win32.hpp"
#include "gui_system.hpp"
#include "keyboard_virtual_key.hpp"
#include "theme_book.hpp"
#include "../GFX/gfx_system_vulkan.hpp"
#include "../widgets/window_widget.hpp"
#include "../log.hpp"
#include "../strings.hpp"
#include "../thread.hpp"
#include <windowsx.h>
#include <dwmapi.h>
#include <new>

#pragma comment(lib, "dwmapi")

namespace tt {

using namespace std;

static const wchar_t *win32WindowClassName = nullptr;
static WNDCLASSW win32WindowClass = {};
static bool win32WindowClassIsRegistered = false;
static bool firstWindowHasBeenOpened = false;

/** The win32 window message handler.
 * This function should not take any locks as _WindowProc is called recursively.
 */
static LRESULT CALLBACK _WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
{
    if (uMsg == WM_CREATE && lParam) {
        ttlet createData = std::launder(std::bit_cast<CREATESTRUCT *>(lParam));

        SetLastError(0);
        auto r = SetWindowLongPtrW(hwnd, GWLP_USERDATA, std::bit_cast<LONG_PTR>(createData->lpCreateParams));
        if (r != 0 || GetLastError() != 0) {
            tt_log_fatal("Could not set GWLP_USERDATA on window. '{}'", get_last_error_message());
        }
    }

    // It is assumed that GWLP_USERDATA is zero when the window is created. Because messages to
    // this window are send before WM_CREATE and there is no way to figure out to which actual window
    // these messages belong.
    auto window_userdata = GetWindowLongPtrW(hwnd, GWLP_USERDATA);
    if (window_userdata == 0) {
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    auto window = std::launder(std::bit_cast<gui_window_win32 *>(window_userdata));
    tt_axiom(window->is_gui_thread());

    LRESULT result = window->windowProc(uMsg, wParam, lParam);

    if (uMsg == WM_DESTROY) {
        // Remove the window now, before DefWindowProc, which could recursively
        // Reuse the window as it is being cleaned up.
        SetLastError(0);
        auto r = SetWindowLongPtrW(hwnd, GWLP_USERDATA, NULL);
        if (r == 0 || GetLastError() != 0) {
            tt_log_fatal("Could not set GWLP_USERDATA on window. '{}'", get_last_error_message());
        }
    }

    // The call to DefWindowProc() recurses make sure we do not hold on to any locks.
    if (result == -1) {
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return result;
}

static void createWindowClass()
{
    if (!win32WindowClassIsRegistered) {
        // Register the window class.
        win32WindowClassName = L"TTauri Window Class";

        std::memset(&win32WindowClass, 0, sizeof(WNDCLASSW));
        win32WindowClass.style = CS_DBLCLKS;
        win32WindowClass.lpfnWndProc = _WindowProc;
        win32WindowClass.hInstance = reinterpret_cast<HINSTANCE>(gui_system::instance);
        win32WindowClass.lpszClassName = win32WindowClassName;
        win32WindowClass.hCursor = nullptr;
        RegisterClassW(&win32WindowClass);
    }
    win32WindowClassIsRegistered = true;
}

void gui_window_win32::create_window(extent2 new_size)
{
    // This function should be called during init(), and therefor should not have a lock on the window.
    tt_assert(is_gui_thread());

    createWindowClass();

    auto u16title = to_wstring(title.text());

    tt_log_info("Create window of size {} with title '{}'", new_size, title);

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
        narrow_cast<int>(new_size.width()),
        narrow_cast<int>(new_size.height()),

        NULL, // Parent window
        NULL, // Menu
        reinterpret_cast<HINSTANCE>(gui_system::instance), // Instance handle
        this);
    if (win32Window == nullptr) {
        tt_log_fatal("Could not open a win32 window: {}", get_last_error_message());
    }

    // Now we extend the drawable area over the titlebar and and border, excluding the drop shadow.
    // At least one value needs to be postive for the drop-shadow to be rendered.
    MARGINS m{0, 0, 0, 1};
    DwmExtendFrameIntoClientArea(reinterpret_cast<HWND>(win32Window), &m);

    // Force WM_NCCALCSIZE to be send to the window.
    SetWindowPos(
        reinterpret_cast<HWND>(win32Window),
        nullptr,
        0,
        0,
        0,
        0,
        SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

    if (!firstWindowHasBeenOpened) {
        ttlet win32_window_ = reinterpret_cast<HWND>(win32Window);
        switch (gui_window_size::normal) {
        case gui_window_size::normal: ShowWindow(win32_window_, SW_SHOWNORMAL); break;
        case gui_window_size::minimized: ShowWindow(win32_window_, SW_SHOWMINIMIZED); break;
        case gui_window_size::maximized: ShowWindow(win32_window_, SW_SHOWMAXIMIZED); break;
        default: tt_no_default();
        }
        firstWindowHasBeenOpened = true;
    }

    trackMouseLeaveEventParameters.cbSize = sizeof(trackMouseLeaveEventParameters);
    trackMouseLeaveEventParameters.dwFlags = TME_LEAVE;
    trackMouseLeaveEventParameters.hwndTrack = reinterpret_cast<HWND>(win32Window);
    trackMouseLeaveEventParameters.dwHoverTime = HOVER_DEFAULT;

    ShowWindow(reinterpret_cast<HWND>(win32Window), SW_SHOW);

    auto _dpi = GetDpiForWindow(reinterpret_cast<HWND>(win32Window));
    if (_dpi == 0) {
        throw gui_error("Could not retrieve dpi for window.");
    }
    dpi = narrow_cast<float>(_dpi);

    surface = gui.gfx->make_surface(gui_system::instance, win32Window);
}

gui_window_win32::gui_window_win32(gui_system &gui, label const &title, std::weak_ptr<gui_window_delegate> delegate) noexcept :
    gui_window(gui, title, std::move(delegate)), trackMouseLeaveEventParameters()
{
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    doubleClickMaximumDuration = GetDoubleClickTime() * 1ms;
    tt_log_info("Double click duration {} ms", doubleClickMaximumDuration / 1ms);
}

gui_window_win32::~gui_window_win32()
{
    try {
        if (win32Window != nullptr) {
            tt_log_fatal("win32Window was not destroyed before Window '{}' was destructed.", title);
        }

    } catch (std::exception const &e) {
        tt_log_fatal("Could not properly destruct gui_window_win32. '{}'", e.what());
    }
}

void gui_window_win32::close_window()
{
    gui.run_from_event_queue([=]() {
        DestroyWindow(reinterpret_cast<HWND>(win32Window));
    });
}

void gui_window_win32::minimize_window()
{
    gui.run_from_event_queue([=]() {
        ShowWindow(reinterpret_cast<HWND>(win32Window), SW_MINIMIZE);
    });
}

void gui_window_win32::maximize_window()
{
    gui.run_from_event_queue([=]() {
        ShowWindow(reinterpret_cast<HWND>(win32Window), SW_MAXIMIZE);
    });
}

void gui_window_win32::normalize_window()
{
    gui.run_from_event_queue([=]() {
        ShowWindow(reinterpret_cast<HWND>(win32Window), SW_RESTORE);
    });
}

void gui_window_win32::set_window_size(extent2 new_extent)
{
    tt_axiom(is_gui_thread());
    ttlet handle = reinterpret_cast<HWND>(win32Window);

    SetWindowPos(
        reinterpret_cast<HWND>(handle),
        HWND_NOTOPMOST,
        0,
        0,
        narrow_cast<int>(std::ceil(new_extent.width())),
        narrow_cast<int>(std::ceil(new_extent.height())),
        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOREDRAW | SWP_DEFERERASE | SWP_NOCOPYBITS | SWP_FRAMECHANGED);
}

[[nodiscard]] extent2 gui_window_win32::virtual_screen_size() const noexcept
{
    ttlet width = GetSystemMetrics(SM_CXMAXTRACK);
    ttlet height = GetSystemMetrics(SM_CYMAXTRACK);
    if (width <= 0 || height <= 0) {
        tt_log_fatal("Failed to get virtual screen size");
    }
    return {narrow_cast<float>(width), narrow_cast<float>(height)};
}

[[nodiscard]] std::string gui_window_win32::get_text_from_clipboard() const noexcept
{
    tt_axiom(is_gui_thread());

    auto r = std::string{};

    ttlet handle = reinterpret_cast<HWND>(win32Window);

    if (!OpenClipboard(handle)) {
        tt_log_error("Could not open win32 clipboard '{}'", get_last_error_message());
        return r;
    }

    UINT format = 0;

    while ((format = EnumClipboardFormats(format)) != 0) {
        switch (format) {
        case CF_TEXT:
        case CF_OEMTEXT:
        case CF_UNICODETEXT: {
            ttlet cb_data = GetClipboardData(CF_UNICODETEXT);
            if (cb_data == nullptr) {
                tt_log_error("Could not get clipboard data: '{}'", get_last_error_message());
                goto done;
            }

            ttlet wstr_c = reinterpret_cast<wchar_t *>(GlobalLock(cb_data));
            if (wstr_c == nullptr) {
                tt_log_error("Could not lock clipboard data: '{}'", get_last_error_message());
                goto done;
            }

            ttlet wstr = std::wstring_view(wstr_c);
            r = tt::to_string(wstr);
            tt_log_debug("getTextFromClipboad '{}'", r);

            if (!GlobalUnlock(cb_data) && GetLastError() != ERROR_SUCCESS) {
                tt_log_error("Could not unlock clipboard data: '{}'", get_last_error_message());
                goto done;
            }
        }
            goto done;

        default:;
        }
    }

    if (GetLastError() != ERROR_SUCCESS) {
        tt_log_error("Could not enumerator clipboard formats: '{}'", get_last_error_message());
    }

done:
    CloseClipboard();

    return r;
}

void gui_window_win32::set_text_on_clipboard(std::string str) noexcept
{
    if (!OpenClipboard(reinterpret_cast<HWND>(win32Window))) {
        tt_log_error("Could not open win32 clipboard '{}'", get_last_error_message());
        return;
    }

    if (!EmptyClipboard()) {
        tt_log_error("Could not empty win32 clipboard '{}'", get_last_error_message());
        goto done;
    }

    {
        auto wstr = tt::to_wstring(str);

        auto wstr_handle = GlobalAlloc(GMEM_MOVEABLE, (std::ssize(wstr) + 1) * sizeof(wchar_t));
        if (wstr_handle == nullptr) {
            tt_log_error("Could not allocate clipboard data '{}'", get_last_error_message());
            goto done;
        }

        auto wstr_c = reinterpret_cast<wchar_t *>(GlobalLock(wstr_handle));
        if (wstr_c == nullptr) {
            tt_log_error("Could not lock clipboard data '{}'", get_last_error_message());
            GlobalFree(wstr_handle);
            goto done;
        }

        std::memcpy(wstr_c, wstr.c_str(), (std::ssize(wstr) + 1) * sizeof(wchar_t));

        if (!GlobalUnlock(wstr_handle) && GetLastError() != ERROR_SUCCESS) {
            tt_log_error("Could not unlock clipboard data '{}'", get_last_error_message());
            GlobalFree(wstr_handle);
            goto done;
        }

        auto handle = SetClipboardData(CF_UNICODETEXT, wstr_handle);
        if (handle == nullptr) {
            tt_log_error("Could not set clipboard data '{}'", get_last_error_message());
            GlobalFree(wstr_handle);
            goto done;
        }
    }

done:
    CloseClipboard();
}

void gui_window_win32::setOSWindowRectangleFromRECT(RECT rectangle) noexcept
{
    tt_axiom(is_gui_thread());

    auto screen_extent = virtual_screen_size();

    screen_rectangle = aarectangle{
        narrow_cast<float>(rectangle.left),
        narrow_cast<float>(screen_extent.height() - rectangle.bottom),
        narrow_cast<float>(rectangle.right - rectangle.left),
        narrow_cast<float>(rectangle.bottom - rectangle.top)};

    request_relayout();
}

void gui_window_win32::set_cursor(mouse_cursor cursor) noexcept
{
    tt_axiom(is_gui_thread());

    if (currentmouse_cursor == cursor) {
        return;
    }
    currentmouse_cursor = cursor;

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
    case mouse_cursor::None: idc = idcAppStarting; break;
    case mouse_cursor::Default: idc = idcArrow; break;
    case mouse_cursor::Button: idc = idcHand; break;
    case mouse_cursor::TextEdit: idc = idcIBeam; break;
    default: tt_no_default();
    }

    SetCursor(idc);
}

[[nodiscard]] keyboard_modifiers gui_window_win32::getkeyboard_modifiers() noexcept
{
    auto r = keyboard_modifiers::None;

    if ((static_cast<uint16_t>(GetAsyncKeyState(VK_SHIFT)) & 0x8000) != 0) {
        r |= keyboard_modifiers::Shift;
    }
    if ((static_cast<uint16_t>(GetAsyncKeyState(VK_CONTROL)) & 0x8000) != 0) {
        r |= keyboard_modifiers::Control;
    }
    if ((static_cast<uint16_t>(GetAsyncKeyState(VK_MENU)) & 0x8000) != 0) {
        r |= keyboard_modifiers::Alt;
    }
    if ((static_cast<uint16_t>(GetAsyncKeyState(VK_LWIN)) & 0x8000) != 0 ||
        (static_cast<uint16_t>(GetAsyncKeyState(VK_RWIN)) & 0x8000) != 0) {
        r |= keyboard_modifiers::Super;
    }

    return r;
}

[[nodiscard]] KeyboardState gui_window_win32::getKeyboardState() noexcept
{
    auto r = KeyboardState::Idle;

    if (GetKeyState(VK_CAPITAL) != 0) {
        r |= KeyboardState::CapsLock;
    }
    if (GetKeyState(VK_NUMLOCK) != 0) {
        r |= KeyboardState::NumLock;
    }
    if (GetKeyState(VK_SCROLL) != 0) {
        r |= KeyboardState::ScrollLock;
    }
    return r;
}

/** The win32 window message handler.
 * This function should not take any long-term-locks as windowProc is called recursively.
 */
int gui_window_win32::windowProc(unsigned int uMsg, uint64_t wParam, int64_t lParam) noexcept
{
    mouse_event mouseEvent;
    ttlet current_time = std::chrono::utc_clock::now();

    switch (uMsg) {
    case WM_DESTROY: {
        tt_axiom(is_gui_thread());
        surface->set_closed();
        win32Window = nullptr;
    } break;

    case WM_CREATE: {
        ttlet createstruct_ptr = std::launder(std::bit_cast<CREATESTRUCT *>(lParam));
        RECT rectangle;
        rectangle.left = createstruct_ptr->x;
        rectangle.top = createstruct_ptr->y;
        rectangle.right = createstruct_ptr->x + createstruct_ptr->cx;
        rectangle.bottom = createstruct_ptr->y + createstruct_ptr->cy;
        setOSWindowRectangleFromRECT(rectangle);
    } break;

    case WM_ERASEBKGND: return 1;

    case WM_PAINT: {
        ttlet height = [this]() {
            tt_axiom(is_gui_thread());
            return screen_rectangle.height();
        }();

        PAINTSTRUCT ps;
        BeginPaint(win32Window, &ps);

        ttlet update_rectangle = aarectangle{
            narrow_cast<float>(ps.rcPaint.left),
            narrow_cast<float>(height - ps.rcPaint.bottom),
            narrow_cast<float>(ps.rcPaint.right - ps.rcPaint.left),
            narrow_cast<float>(ps.rcPaint.bottom - ps.rcPaint.top)};

        {
            tt_axiom(is_gui_thread());
            request_redraw(update_rectangle);
        }

        EndPaint(win32Window, &ps);
    } break;

    case WM_NCPAINT: {
        tt_axiom(is_gui_thread());
        request_redraw();
    } break;

    case WM_SIZE: {
        tt_axiom(is_gui_thread());
        switch (wParam) {
        case SIZE_MAXIMIZED: size_state = gui_window_size::maximized; break;
        case SIZE_MINIMIZED: size_state = gui_window_size::minimized; break;
        case SIZE_RESTORED: size_state = gui_window_size::normal; break;
        default: break;
        }
    } break;

    case WM_TIMER: {
        if (last_forced_redraw + 16.7ms < current_time) {
            // During sizing the event loop is blocked.
            // Render at about 60fps.
            gui.render(current_time);
            last_forced_redraw = current_time;
        }
    } break;

    case WM_SIZING: {
        ttlet rect_ptr = std::launder(std::bit_cast<RECT *>(lParam));
        setOSWindowRectangleFromRECT(*rect_ptr);
    } break;

    case WM_MOVING: {
        ttlet rect_ptr = std::launder(std::bit_cast<RECT *>(lParam));
        setOSWindowRectangleFromRECT(*rect_ptr);
    } break;

    case WM_WINDOWPOSCHANGED: {
        ttlet windowpos_ptr = std::launder(std::bit_cast<WINDOWPOS *>(lParam));
        RECT rectangle;
        rectangle.left = windowpos_ptr->x;
        rectangle.top = windowpos_ptr->y;
        rectangle.right = windowpos_ptr->x + windowpos_ptr->cx;
        rectangle.bottom = windowpos_ptr->y + windowpos_ptr->cy;
        setOSWindowRectangleFromRECT(rectangle);
    } break;

    case WM_ENTERSIZEMOVE: {
        tt_axiom(is_gui_thread());
        if (SetTimer(win32Window, move_and_resize_timer_id, 16, NULL) != move_and_resize_timer_id) {
            tt_log_error("Could not set timer before move/resize. {}", get_last_error_message());
        }
        resizing = true;
    } break;

    case WM_EXITSIZEMOVE: {
        tt_axiom(is_gui_thread());
        if (not KillTimer(win32Window, move_and_resize_timer_id)) {
            tt_log_error("Could not kill timer after move/resize. {}", get_last_error_message());
        }
        resizing = false;
    } break;

    case WM_ACTIVATE: {
        tt_axiom(is_gui_thread());
        switch (wParam) {
        case 1: // WA_ACTIVE
        case 2: // WA_CLICKACTIVE
            active = true;
            break;
        case 0: // WA_INACTIVE
            active = false;
            break;
        default: tt_log_error("Unknown WM_ACTIVE value.");
        }
        request_relayout();
    } break;

    case WM_GETMINMAXINFO: {
        tt_axiom(is_gui_thread());
        tt_axiom(widget);
        ttlet minimum_widget_size = widget->constraints().minimum;
        ttlet maximum_widget_size = widget->constraints().maximum;
        ttlet minmaxinfo = std::launder(std::bit_cast<MINMAXINFO *>(lParam));
        minmaxinfo->ptMaxSize.x = narrow_cast<LONG>(maximum_widget_size.width());
        minmaxinfo->ptMaxSize.y = narrow_cast<LONG>(maximum_widget_size.height());
        minmaxinfo->ptMinTrackSize.x = narrow_cast<LONG>(minimum_widget_size.width());
        minmaxinfo->ptMinTrackSize.y = narrow_cast<LONG>(minimum_widget_size.height());
        minmaxinfo->ptMaxTrackSize.x = narrow_cast<LONG>(maximum_widget_size.width());
        minmaxinfo->ptMaxTrackSize.y = narrow_cast<LONG>(maximum_widget_size.height());
    } break;

    case WM_UNICHAR: {
        auto c = narrow_cast<char32_t>(wParam);
        if (c == UNICODE_NOCHAR) {
            // Tell the 3rd party keyboard handler application that we support WM_UNICHAR.
            return 1;
        } else if (c >= 0x20) {
            auto keyboardEvent = keyboard_event();
            keyboardEvent.type = keyboard_event::Type::grapheme;
            keyboardEvent.grapheme = c;
            send_event(keyboardEvent);
        }
    } break;

    case WM_DEADCHAR: {
        auto c = handleSuragates(narrow_cast<char32_t>(wParam));
        if (c != 0) {
            send_event(c, false);
        }
    } break;

    case WM_CHAR: {
        auto c = handleSuragates(narrow_cast<char32_t>(wParam));
        if (c >= 0x20) {
            send_event(c);
        }
    } break;

    case WM_SYSKEYDOWN: {
        auto alt_pressed = (narrow_cast<uint32_t>(lParam) & 0x20000000) != 0;
        if (!alt_pressed) {
            return -1;
        }
    }
        [[fallthrough]];
    case WM_KEYDOWN: {
        auto extended = (narrow_cast<uint32_t>(lParam) & 0x01000000) != 0;
        auto key_code = narrow_cast<int>(wParam);

        ttlet key_state = getKeyboardState();
        ttlet key_modifiers = getkeyboard_modifiers();
        ttlet virtual_key = to_keyboard_virtual_key(key_code, extended, key_modifiers);
        if (virtual_key != keyboard_virtual_key::Nul) {
            send_event(key_state, key_modifiers, virtual_key);
        }
    } break;

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
    case WM_MOUSELEAVE: send_event(createmouse_event(uMsg, wParam, lParam)); break;

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

    case WM_NCHITTEST: {
        tt_axiom(is_gui_thread());

        ttlet screen_extent = virtual_screen_size();
        ttlet screen_position =
            point2(narrow_cast<float>(GET_X_LPARAM(lParam)), screen_extent.height() - narrow_cast<float>(GET_Y_LPARAM(lParam)));

        ttlet hitbox_type = widget->hitbox_test(screen_to_window() * screen_position).type;

        switch (hitbox_type) {
        case hitbox::Type::BottomResizeBorder: set_cursor(mouse_cursor::None); return HTBOTTOM;
        case hitbox::Type::TopResizeBorder: set_cursor(mouse_cursor::None); return HTTOP;
        case hitbox::Type::LeftResizeBorder: set_cursor(mouse_cursor::None); return HTLEFT;
        case hitbox::Type::RightResizeBorder: set_cursor(mouse_cursor::None); return HTRIGHT;
        case hitbox::Type::BottomLeftResizeCorner: set_cursor(mouse_cursor::None); return HTBOTTOMLEFT;
        case hitbox::Type::BottomRightResizeCorner: set_cursor(mouse_cursor::None); return HTBOTTOMRIGHT;
        case hitbox::Type::TopLeftResizeCorner: set_cursor(mouse_cursor::None); return HTTOPLEFT;
        case hitbox::Type::TopRightResizeCorner: set_cursor(mouse_cursor::None); return HTTOPRIGHT;
        case hitbox::Type::ApplicationIcon: set_cursor(mouse_cursor::None); return HTSYSMENU;
        case hitbox::Type::MoveArea: set_cursor(mouse_cursor::None); return HTCAPTION;
        case hitbox::Type::TextEdit: set_cursor(mouse_cursor::TextEdit); return HTCLIENT;
        case hitbox::Type::Button: set_cursor(mouse_cursor::Button); return HTCLIENT;
        case hitbox::Type::Default: set_cursor(mouse_cursor::Default); return HTCLIENT;
        case hitbox::Type::Outside: set_cursor(mouse_cursor::None); return HTCLIENT;
        default: tt_no_default();
        }
    } break;

    case WM_SETTINGCHANGE: {
        tt_axiom(is_gui_thread());
        doubleClickMaximumDuration = GetDoubleClickTime() * 1ms;
        tt_log_info("Double click duration {} ms", doubleClickMaximumDuration / 1ms);

        gui.set_theme_mode(read_os_theme_mode());
        request_reconstrain();
    } break;

    case WM_DPICHANGED: {
        tt_axiom(is_gui_thread());
        // x-axis dpi value.
        dpi = narrow_cast<float>(LOWORD(wParam));
        tt_log_info("DPI has changed to {}", dpi);
        request_relayout();
    } break;

    default: break;
    }

    // Let DefWindowProc() handle it.
    return -1;
}

[[nodiscard]] char32_t gui_window_win32::handleSuragates(char32_t c) noexcept
{
    tt_axiom(is_gui_thread());

    if (c >= 0xd800 && c <= 0xdbff) {
        highSurrogate = ((c - 0xd800) << 10) + 0x10000;
        return 0;

    } else if (c >= 0xdc00 && c <= 0xdfff) {
        c = highSurrogate ? highSurrogate | (c - 0xdc00) : 0xfffd;
    }
    highSurrogate = 0;
    return c;
}

[[nodiscard]] mouse_event gui_window_win32::createmouse_event(unsigned int uMsg, uint64_t wParam, int64_t lParam) noexcept
{
    tt_axiom(is_gui_thread());

    auto mouseEvent = mouse_event{};
    mouseEvent.timePoint = std::chrono::utc_clock::now();

    // On Window 7 up to and including Window10, the I-beam cursor hot-spot is 2 pixels to the left
    // of the vertical bar. But most applications do not fix this problem.
    mouseEvent.position =
        point2(narrow_cast<float>(GET_X_LPARAM(lParam)), screen_rectangle.height() - narrow_cast<float>(GET_Y_LPARAM(lParam)));

    mouseEvent.wheelDelta = {};
    if (uMsg == WM_MOUSEWHEEL) {
        mouseEvent.wheelDelta.y() = narrow_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / WHEEL_DELTA * 10.0f;
    } else if (uMsg == WM_MOUSEHWHEEL) {
        mouseEvent.wheelDelta.x() = narrow_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / WHEEL_DELTA * 10.0f;
    }

    // Track which buttons are down, in case the application wants to track multiple buttons being pressed down.
    mouseEvent.down.controlKey = (GET_KEYSTATE_WPARAM(wParam) & MK_CONTROL) > 0;
    mouseEvent.down.leftButton = (GET_KEYSTATE_WPARAM(wParam) & MK_LBUTTON) > 0;
    mouseEvent.down.middleButton = (GET_KEYSTATE_WPARAM(wParam) & MK_MBUTTON) > 0;
    mouseEvent.down.rightButton = (GET_KEYSTATE_WPARAM(wParam) & MK_RBUTTON) > 0;
    mouseEvent.down.shiftKey = (GET_KEYSTATE_WPARAM(wParam) & MK_SHIFT) > 0;
    mouseEvent.down.x1Button = (GET_KEYSTATE_WPARAM(wParam) & MK_XBUTTON1) > 0;
    mouseEvent.down.x2Button = (GET_KEYSTATE_WPARAM(wParam) & MK_XBUTTON2) > 0;

    // Check which buttons caused the mouse event.
    switch (uMsg) {
    case WM_LBUTTONUP:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK: mouseEvent.cause.leftButton = true; break;
    case WM_RBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONDBLCLK: mouseEvent.cause.rightButton = true; break;
    case WM_MBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONDBLCLK: mouseEvent.cause.middleButton = true; break;
    case WM_XBUTTONUP:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONDBLCLK:
        mouseEvent.cause.x1Button = (GET_XBUTTON_WPARAM(wParam) & XBUTTON1) > 0;
        mouseEvent.cause.x2Button = (GET_XBUTTON_WPARAM(wParam) & XBUTTON2) > 0;
        break;
    case WM_MOUSEMOVE:
        if (mouseButtonEvent.type == mouse_event::Type::ButtonDown) {
            mouseEvent.cause = mouseButtonEvent.cause;
        }
        break;
    case WM_MOUSEWHEEL:
    case WM_MOUSEHWHEEL:
    case WM_MOUSELEAVE: break;
    default: tt_no_default();
    }

    ttlet a_button_is_pressed = mouseEvent.down.leftButton || mouseEvent.down.middleButton || mouseEvent.down.rightButton ||
        mouseEvent.down.x1Button || mouseEvent.down.x2Button;

    switch (uMsg) {
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    case WM_XBUTTONUP:
        mouseEvent.type = mouse_event::Type::ButtonUp;
        mouseEvent.downPosition = mouseButtonEvent.downPosition;
        mouseEvent.clickCount = 0;

        if (!a_button_is_pressed) {
            ReleaseCapture();
        }
        break;

    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_XBUTTONDOWN: {
        mouseEvent.type = mouse_event::Type::ButtonDown;
        mouseEvent.downPosition = mouseEvent.position;
        mouseEvent.clickCount = (mouseEvent.timePoint < doubleClickTimePoint + doubleClickMaximumDuration) ? 3 : 1;

        // Track draging past the window borders.
        tt_axiom(win32Window != 0);
        ttlet window_handle = reinterpret_cast<HWND>(win32Window);

        SetCapture(window_handle);
    } break;

    case WM_LBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
    case WM_XBUTTONDBLCLK:
        mouseEvent.type = mouse_event::Type::ButtonDown;
        mouseEvent.downPosition = mouseEvent.position;
        mouseEvent.clickCount = 2;
        doubleClickTimePoint = std::chrono::utc_clock::now();
        break;

    case WM_MOUSEWHEEL:
    case WM_MOUSEHWHEEL: mouseEvent.type = mouse_event::Type::Wheel; break;

    case WM_MOUSEMOVE: {
        // XXX Make sure the mouse is moved enough for this to cause a drag event.
        mouseEvent.type = a_button_is_pressed ? mouse_event::Type::Drag : mouse_event::Type::Move;
        mouseEvent.downPosition = mouseButtonEvent.downPosition;
        mouseEvent.clickCount = mouseButtonEvent.clickCount;
    } break;

    case WM_MOUSELEAVE:
        mouseEvent.type = mouse_event::Type::Exited;
        mouseEvent.downPosition = mouseButtonEvent.downPosition;
        mouseEvent.clickCount = 0;

        // After this event we need to ask win32 to track the mouse again.
        trackingMouseLeaveEvent = false;

        // Force currentmouse_cursor to None so that the Window is in a fresh
        // state when the mouse reenters it.
        currentmouse_cursor = mouse_cursor::None;
        break;

    default: tt_no_default();
    }

    // Make sure we start tracking mouse events when the mouse has entered the window again.
    // So that once the mouse leaves the window we receive a WM_MOUSELEAVE event.
    if (!trackingMouseLeaveEvent && uMsg != WM_MOUSELEAVE) {
        auto *track_mouse_leave_event_parameters_p = &trackMouseLeaveEventParameters;
        if (!TrackMouseEvent(track_mouse_leave_event_parameters_p)) {
            tt_log_error("Could not track leave event '{}'", get_last_error_message());
        }
        trackingMouseLeaveEvent = true;
    }

    // Remember the last time a button was pressed or released, so that we can convert
    // a move into a drag event.
    if (mouseEvent.type == mouse_event::Type::ButtonDown || mouseEvent.type == mouse_event::Type::ButtonUp ||
        mouseEvent.type == mouse_event::Type::Exited) {
        mouseButtonEvent = mouseEvent;
    }

    return mouseEvent;
}

} // namespace tt
