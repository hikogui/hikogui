// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/GUI/Window_vulkan_win32.hpp"
#include "TTauri/GUI/KeyboardVirtualKey_win32.hpp"
#include "TTauri/GUI/Instance.hpp"
#include "TTauri/Foundation/strings.hpp"
#include "TTauri/Foundation/thread.hpp"
#include <windowsx.h>
#include <dwmapi.h>

#pragma comment( lib, "dwmapi" )

namespace TTauri::GUI {

using namespace std;
using namespace TTauri;

template<typename T>
inline gsl::not_null<T *>to_ptr(LPARAM lParam) noexcept
{
    T *ptr;
    memcpy(&ptr, &lParam, sizeof(T *));

    return gsl::not_null<T *>(ptr);
}

static const wchar_t *win32WindowClassName = nullptr;
static WNDCLASSW win32WindowClass = {};
static bool win32WindowClassIsRegistered = false;
static std::unordered_map<HWND, Window_vulkan_win32 *> win32WindowMap = {};
static bool firstWindowHasBeenOpened = false;


gsl_suppress3(26489,lifetime.1,type.1)
static LRESULT CALLBACK _WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_NCCREATE && lParam) {
        let createData = reinterpret_cast<CREATESTRUCT *>(lParam);

        if (createData->lpCreateParams) {
            win32WindowMap[hwnd] = static_cast<Window_vulkan_win32 *>(createData->lpCreateParams);
        }
    }

    auto i = win32WindowMap.find(hwnd);
    if (i != win32WindowMap.end()) {
        let window = i->second;

        LRESULT result = window->windowProc(uMsg, wParam, lParam);
        if (result == -1) {
            result = DefWindowProc(hwnd, uMsg, wParam, lParam);
        }

        if (uMsg == WM_DESTROY) {
            win32WindowMap.erase(i);
        }

        return result;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

static void createWindowClass()
{
    if (!win32WindowClassIsRegistered) {
         // Register the window class.
        win32WindowClassName = L"TTauri Window Class";

        std::memset(&win32WindowClass, 0, sizeof(WNDCLASSW));
        win32WindowClass.style = CS_DBLCLKS;
        win32WindowClass.lpfnWndProc = _WindowProc;
        win32WindowClass.hInstance = reinterpret_cast<HINSTANCE>(hInstance);
        win32WindowClass.lpszClassName = win32WindowClassName;
        win32WindowClass.hCursor = nullptr;
        RegisterClassW(&win32WindowClass);
    }
    win32WindowClassIsRegistered = true;
}

void Window_vulkan_win32::createWindow(const std::string &title, vec extent)
{
    createWindowClass();

    auto u16title = to_wstring(title);

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
        numeric_cast<int>(extent.x()),
        numeric_cast<int>(extent.y()),

        NULL, // Parent window
        NULL, // Menu
        reinterpret_cast<HINSTANCE>(hInstance), // Instance handle
        this
    );

    // Now we extend the drawable area over the titlebar and and border, excluding the drop shadow.
    MARGINS m{ 0, 0, 0, 1 };
    DwmExtendFrameIntoClientArea(reinterpret_cast<HWND>(win32Window), &m);

    // Force WM_NCCALCSIZE to be send to the window.
    SetWindowPos(reinterpret_cast<HWND>(win32Window), nullptr, 0, 0, 0, 0, SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);

    if (win32Window == nullptr) {
        TTAURI_THROW(gui_error("Could not open a win32 window."));
    }

    if (!firstWindowHasBeenOpened) {
        ShowWindow(reinterpret_cast<HWND>(win32Window), nCmdShow);
        firstWindowHasBeenOpened = true;
    }

    trackMouseLeaveEventParameters.cbSize = sizeof(trackMouseLeaveEventParameters);
    trackMouseLeaveEventParameters.dwFlags = TME_LEAVE;
    trackMouseLeaveEventParameters.hwndTrack = reinterpret_cast<HWND>(win32Window);
    trackMouseLeaveEventParameters.dwHoverTime = HOVER_DEFAULT;

    ShowWindow(reinterpret_cast<HWND>(win32Window), SW_SHOW);
}

Window_vulkan_win32::Window_vulkan_win32(const std::shared_ptr<WindowDelegate> delegate, const std::string title) :
    Window_vulkan(move(delegate), title),
    trackMouseLeaveEventParameters()
{
}

Window_vulkan_win32::~Window_vulkan_win32()
{
    try {
        gsl_suppress(f.6) {
            if (win32Window != nullptr) {
                LOG_FATAL("win32Window was not destroyed before Window '{}' was destructed.", title);
                abort();
            }
        }
    } catch (...) {
        abort();
    }
}

void Window_vulkan_win32::closeWindow()
{
    run_on_main_thread([&]() {
        DestroyWindow(reinterpret_cast<HWND>(win32Window));
    });
}

void Window_vulkan_win32::minimizeWindow()
{
    run_on_main_thread([&]() {
        ShowWindow(reinterpret_cast<HWND>(win32Window), SW_MINIMIZE);
    });
}

void Window_vulkan_win32::maximizeWindow()
{
    run_on_main_thread([&]() {
        ShowWindow(reinterpret_cast<HWND>(win32Window), SW_MAXIMIZE);
    });
}

void Window_vulkan_win32::normalizeWindow()
{
    run_on_main_thread([&]() {
        ShowWindow(reinterpret_cast<HWND>(win32Window), SW_RESTORE);
    });
}

void Window_vulkan_win32::closingWindow()
{
    run_on_main_thread([&]() {
        Window_vulkan::closingWindow();
    });
}

void Window_vulkan_win32::openingWindow()
{
    run_on_main_thread([&]() {
        auto lock = std::scoped_lock(guiMutex);

        Window_vulkan::openingWindow();

        // Delegate has been called, layout of widgets has been calculated for the
        // minimum and maximum size of the window.
        createWindow(title, minimumWindowExtent);
    });
}

[[nodiscard]] std::string Window_vulkan_win32::getTextFromClipboard() const noexcept
{
    auto r = std::string{};

    if (!OpenClipboard(reinterpret_cast<HWND>(win32Window))) {
        LOG_ERROR("Could not open win32 clipboard '{}'", getLastErrorMessage());
        return r;
    }

    UINT format = 0;

    while (format = EnumClipboardFormats(format)) {
        switch (format) {
        case CF_TEXT:
        case CF_OEMTEXT:
        case CF_UNICODETEXT: {
            let cb_data = GetClipboardData(CF_UNICODETEXT);
            if (cb_data == nullptr) {
                LOG_ERROR("Could not get clipboard data: '{}'", getLastErrorMessage());
                goto done;
            }

            let wstr_c = reinterpret_cast<wchar_t *>(GlobalLock(cb_data));
            if (wstr_c == nullptr) {
                LOG_ERROR("Could not lock clipboard data: '{}'", getLastErrorMessage());
                goto done;
            }

            let wstr = std::wstring_view(wstr_c);
            r = TTauri::to_string(wstr);
            LOG_DEBUG("getTextFromClipboad '{}'", r);

            if (!GlobalUnlock(cb_data) && GetLastError() != ERROR_SUCCESS) {
                LOG_ERROR("Could not unlock clipboard data: '{}'", getLastErrorMessage());
                goto done;
            }

            } goto done;

        default:;
        }
    }

    if (GetLastError() != ERROR_SUCCESS) {
        LOG_ERROR("Could not enumerator clipboard formats: '{}'", getLastErrorMessage());
    }

done:
    CloseClipboard();

    return r;
}

void Window_vulkan_win32::setTextOnClipboard(std::string str) noexcept
{
    if (!OpenClipboard(reinterpret_cast<HWND>(win32Window))) {
        LOG_ERROR("Could not open win32 clipboard '{}'", getLastErrorMessage());
        return;
    }

    if (!EmptyClipboard()) {
        LOG_ERROR("Could not empty win32 clipboard '{}'", getLastErrorMessage());
        goto done;
    }

    {
        auto wstr = TTauri::to_wstring(str);

        auto wstr_handle = GlobalAlloc(GMEM_MOVEABLE, (ssize(wstr) + 1) * sizeof(wchar_t));
        if (wstr_handle == nullptr) {
            LOG_ERROR("Could not allocate clipboard data '{}'", getLastErrorMessage());
            goto done;
        }

        auto wstr_c = reinterpret_cast<wchar_t *>(GlobalLock(wstr_handle));
        if (wstr_c == nullptr) {
            LOG_ERROR("Could not lock clipboard data '{}'", getLastErrorMessage());
            GlobalFree(wstr_handle);
            goto done;
        }

        std::memcpy(wstr_c, wstr.c_str(), (ssize(wstr) + 1) * sizeof(wchar_t));

        if (!GlobalUnlock(wstr_handle) && GetLastError() != ERROR_SUCCESS) {
            LOG_ERROR("Could not unlock clipboard data '{}'", getLastErrorMessage());
            GlobalFree(wstr_handle);
            goto done;
        }

        auto handle = SetClipboardData(CF_UNICODETEXT, wstr_handle);
        if (handle == nullptr) {
            LOG_ERROR("Could not set clipboard data '{}'", getLastErrorMessage());
            GlobalFree(wstr_handle);
            goto done;
        }
    }

done:
    CloseClipboard();
}


vk::SurfaceKHR Window_vulkan_win32::getSurface() const
{
    return guiSystem->createWin32SurfaceKHR({
        vk::Win32SurfaceCreateFlagsKHR(),
        reinterpret_cast<HINSTANCE>(hInstance),
        reinterpret_cast<HWND>(win32Window)
    });
}

void Window_vulkan_win32::setOSWindowRectangleFromRECT(RECT rect) noexcept
{
    // XXX Without screen height, it is not possible to calculate the y of the left-bottom corner.
    OSWindowRectangle = irect{
        rect.left,
        0 - rect.bottom,
        rect.right - rect.left,
        rect.bottom - rect.top
    };
    widget->handleWindowResize();
}

void Window_vulkan_win32::setCursor(Cursor cursor) noexcept {
    static auto idcAppStarting = LoadCursorW(nullptr, IDC_APPSTARTING);
    static auto idcArrow = LoadCursorW(nullptr, IDC_ARROW);
    static auto idcHand = LoadCursorW(nullptr, IDC_HAND);
    static auto idcIBeam = LoadCursorW(nullptr, IDC_IBEAM);
    static auto idcNo = LoadCursorW(nullptr, IDC_NO);

    if (currentCursor == cursor) {
        return;
    }

    auto idc = idcNo;
    switch (cursor) {
    case Cursor::None: idc = idcAppStarting; break;
    case Cursor::Default: idc = idcArrow; break;
    case Cursor::Button: idc = idcHand; break;
    case Cursor::TextEdit: idc = idcIBeam; break;
    default: no_default;
    }

    SetCursor(idc);
    currentCursor = cursor;
}

[[nodiscard]] KeyboardModifiers Window_vulkan_win32::getKeyboardModifiers() noexcept
{
    auto r = KeyboardModifiers::None;

    if ((static_cast<uint16_t>(GetAsyncKeyState(VK_SHIFT)) & 0x8000) != 0) {
        r |= KeyboardModifiers::Shift;
    }
    if ((static_cast<uint16_t>(GetAsyncKeyState(VK_CONTROL)) & 0x8000) != 0) {
        r |= KeyboardModifiers::Control;
    }
    if ((static_cast<uint16_t>(GetAsyncKeyState(VK_MENU)) & 0x8000) != 0) {
        r |= KeyboardModifiers::Alt;
    }
    if (
        (static_cast<uint16_t>(GetAsyncKeyState(VK_LWIN)) & 0x8000) != 0 ||
        (static_cast<uint16_t>(GetAsyncKeyState(VK_RWIN)) & 0x8000) != 0
    ) {
        r |= KeyboardModifiers::Super;
    }

    return r;
}

[[nodiscard]] KeyboardState Window_vulkan_win32::getKeyboardState() noexcept
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

int Window_vulkan_win32::windowProc(unsigned int uMsg, uint64_t wParam, int64_t lParam)
{
    auto lock = std::scoped_lock(guiMutex);

    MouseEvent mouseEvent;
    KeyboardEvent keyboardEvent;

    switch (uMsg) {    
    case WM_DESTROY:
        win32Window = nullptr;
        state = State::WindowLost;
        break;

    case WM_CREATE: {
        let createstruct_ptr = to_ptr<CREATESTRUCT>(lParam);
        RECT rect;
        rect.left = createstruct_ptr->x;
        rect.top = createstruct_ptr->y;
        rect.right = createstruct_ptr->x + createstruct_ptr->cx;
        rect.bottom = createstruct_ptr->y + createstruct_ptr->cy;
        setOSWindowRectangleFromRECT(rect);
        } break;

    case WM_PAINT:
        widget->handleWindowResize();
        break;

    case WM_SIZE:
        switch (wParam) {
        case SIZE_MAXIMIZED:
            size = Size::Maximized;
            break;
        case SIZE_MINIMIZED:
            size = Size::Minimized;
            break;
        case SIZE_RESTORED:
            size = Size::Normal;
            break;
        default:
            break;
        }
        break;

    case WM_SIZING: {
        let rect_ptr = to_ptr<RECT>(lParam);
        setOSWindowRectangleFromRECT(*rect_ptr);
        } break;

    case WM_MOVING: {
        let rect_ptr = to_ptr<RECT>(lParam);
        setOSWindowRectangleFromRECT(*rect_ptr);
        } break;

    case WM_WINDOWPOSCHANGED: {
        let windowpos_ptr = to_ptr<WINDOWPOS>(lParam);
        RECT rect;
        rect.left = windowpos_ptr->x;
        rect.top = windowpos_ptr->y;
        rect.right = windowpos_ptr->x + windowpos_ptr->cx;
        rect.bottom = windowpos_ptr->y + windowpos_ptr->cy;
        setOSWindowRectangleFromRECT(rect);
        } break;

    case WM_ENTERSIZEMOVE:
        resizing = true;
        break;

    case WM_EXITSIZEMOVE:
        resizing = false;
        break;
    
    case WM_ACTIVATEAPP:
        active = (wParam == TRUE);
        widget->handleWindowResize();
        break;

    case WM_GETMINMAXINFO: {
        let minmaxinfo = to_ptr<MINMAXINFO>(lParam);
        minmaxinfo->ptMaxSize.x = numeric_cast<long>(maximumWindowExtent.x());
        minmaxinfo->ptMaxSize.y = numeric_cast<long>(maximumWindowExtent.y());
        minmaxinfo->ptMinTrackSize.x = numeric_cast<long>(minimumWindowExtent.x());
        minmaxinfo->ptMinTrackSize.y = numeric_cast<long>(minimumWindowExtent.y());
        minmaxinfo->ptMaxTrackSize.x = numeric_cast<long>(maximumWindowExtent.x());
        minmaxinfo->ptMaxTrackSize.y = numeric_cast<long>(maximumWindowExtent.y());
        } break;


    case WM_UNICHAR: {
        auto c = numeric_cast<char32_t>(wParam);
        if (c == UNICODE_NOCHAR) {
            // Tell the 3rd party keyboard handler application that we support WM_UNICHAR.
            return 1;
        } else if (c >= 0x20) {
            auto keyboardEvent = KeyboardEvent();
            keyboardEvent.type = KeyboardEvent::Type::Grapheme;
            keyboardEvent.grapheme = c;
            handleKeyboardEvent(keyboardEvent);
        }
        } break;

    case WM_DEADCHAR: {
        auto c = handle_suragates(numeric_cast<char32_t>(wParam));
        if (c != 0) {
            handleKeyboardEvent(c, false);
        }
        } break;

    case WM_CHAR: {
        auto c = handle_suragates(numeric_cast<char32_t>(wParam));
        if (c >= 0x20) {
            handleKeyboardEvent(c);
        }
        } break;
        
    case WM_SYSKEYDOWN: {
        auto alt_pressed = (numeric_cast<uint32_t>(lParam) & 0x20000000) != 0;
        if (!alt_pressed) {
            return -1;
        }
        } [[fallthrough]];
    case WM_KEYDOWN: {
        auto extended = (numeric_cast<uint32_t>(lParam) & 0x01000000) != 0;
        auto key_code = numeric_cast<int>(wParam);

        LOG_ERROR("Key 0x{:x} extended={}", key_code, extended);

        let state = getKeyboardState();
        let modifiers = getKeyboardModifiers();
        let virtual_key = to_KeyboardVirtualKey(key_code, extended, modifiers);
        if (virtual_key != KeyboardVirtualKey::Nul) {
            handleKeyboardEvent(state, modifiers, virtual_key);
        }
        } break;

    case WM_LBUTTONDOWN:
        clickCount = 1;
        mouseEvent.type = MouseEvent::Type::ButtonDown;
        mouseEvent.cause.leftButton = true;
        goto parseMouseEvent;
    case WM_LBUTTONUP:
        clickCount = 0;
        mouseEvent.type = MouseEvent::Type::ButtonUp;
        mouseEvent.cause.leftButton = true;
        goto parseMouseEvent;
    case WM_LBUTTONDBLCLK:
        clickCount = 2;
        mouseEvent.type = MouseEvent::Type::ButtonDown;
        mouseEvent.cause.leftButton = true;
        goto parseMouseEvent;
    case WM_MBUTTONDOWN:
        clickCount = 1;
        mouseEvent.type = MouseEvent::Type::ButtonDown;
        mouseEvent.cause.middleButton = true;
        goto parseMouseEvent;
    case WM_MBUTTONUP:
        clickCount = 0;
        mouseEvent.type = MouseEvent::Type::ButtonUp;
        mouseEvent.cause.middleButton = true;
        goto parseMouseEvent;
    case WM_MBUTTONDBLCLK:
        clickCount = 2;
        mouseEvent.type = MouseEvent::Type::ButtonDown;
        mouseEvent.cause.middleButton = true;
        goto parseMouseEvent;
    case WM_RBUTTONDOWN:
        clickCount = 1;
        mouseEvent.type = MouseEvent::Type::ButtonDown;
        mouseEvent.cause.rightButton = true;
        goto parseMouseEvent;
    case WM_RBUTTONUP:
        clickCount = 0;
        mouseEvent.type = MouseEvent::Type::ButtonUp;
        mouseEvent.cause.rightButton = true;
        goto parseMouseEvent;
    case WM_RBUTTONDBLCLK:
        clickCount = 2;
        mouseEvent.type = MouseEvent::Type::ButtonDown;
        mouseEvent.cause.rightButton = true;
        goto parseMouseEvent;
    case WM_XBUTTONDOWN:
        clickCount = 1;
        mouseEvent.type = MouseEvent::Type::ButtonDown;
        mouseEvent.cause.x1Button = (GET_XBUTTON_WPARAM(wParam) & XBUTTON1) > 0;
        mouseEvent.cause.x2Button = (GET_XBUTTON_WPARAM(wParam) & XBUTTON2) > 0;
        goto parseMouseEvent;
    case WM_XBUTTONUP:
        clickCount = 0;
        mouseEvent.type = MouseEvent::Type::ButtonUp;
        mouseEvent.cause.x1Button = (GET_XBUTTON_WPARAM(wParam) & XBUTTON1) > 0;
        mouseEvent.cause.x2Button = (GET_XBUTTON_WPARAM(wParam) & XBUTTON2) > 0;
        goto parseMouseEvent;
    case WM_XBUTTONDBLCLK:
        clickCount = 2;
        mouseEvent.type = MouseEvent::Type::ButtonDown;
        mouseEvent.cause.x1Button = (GET_XBUTTON_WPARAM(wParam) & XBUTTON1) > 0;
        mouseEvent.cause.x2Button = (GET_XBUTTON_WPARAM(wParam) & XBUTTON2) > 0;
        goto parseMouseEvent;

    case WM_MOUSEMOVE:
        if (!trackingMouseLeaveEvent) {
            if (!TrackMouseEvent(&trackMouseLeaveEventParameters)) {
                LOG_ERROR("Could not track leave event '{}'", getLastErrorMessage());
            }
            trackingMouseLeaveEvent = true;
        }
        mouseEvent.type = MouseEvent::Type::Move;
        goto parseMouseEvent;

    parseMouseEvent:
        // On Window 7 up to and including Window10, the I-beam cursor hot-spot is 2 pixels to the left
        // of the vertical bar. But most applications do not fix this problem.

        mouseEvent.position = vec::point(GET_X_LPARAM(lParam), currentWindowExtent.y() - GET_Y_LPARAM(lParam));
        mouseEvent.down.controlKey = (GET_KEYSTATE_WPARAM(wParam) & MK_CONTROL) > 0;
        mouseEvent.down.leftButton = (GET_KEYSTATE_WPARAM(wParam) & MK_LBUTTON) > 0;
        mouseEvent.down.middleButton = (GET_KEYSTATE_WPARAM(wParam) & MK_MBUTTON) > 0;
        mouseEvent.down.rightButton = (GET_KEYSTATE_WPARAM(wParam) & MK_RBUTTON) > 0;
        mouseEvent.down.shiftKey = (GET_KEYSTATE_WPARAM(wParam) & MK_SHIFT) > 0;
        mouseEvent.down.x1Button = (GET_KEYSTATE_WPARAM(wParam) & MK_XBUTTON1) > 0;
        mouseEvent.down.x2Button = (GET_KEYSTATE_WPARAM(wParam) & MK_XBUTTON2) > 0;
        mouseEvent.clickCount = clickCount;
        handleMouseEvent(mouseEvent);
        break;

    case WM_MOUSELEAVE:
        // After this event we need to ask win32 to track the mouse again.
        trackingMouseLeaveEvent = false;

        // Force currentCursor to None so that the Window is in a fresh
        // state when the mouse reenters it.
        currentCursor = Cursor::None;

        handleMouseEvent(MouseEvent::exited());
        break;

    case WM_NCCALCSIZE:
        if (wParam == TRUE) {
            // Return zero to preserve the extended client area on the window.

            // Starting with Windows Vista, removing the standard frame by simply
            // returning 0 when the wParam is TRUE does not affect frames that are
            // extended into the client area using the DwmExtendFrameIntoClientArea function.
            // Only the standard frame will be removed.
            return 0;
        }

        break;

    case WM_NCHITTEST: {
        let screenPosition = vec{
            GET_X_LPARAM(lParam),
            0.0 - GET_Y_LPARAM(lParam)
        };

        let insideWindowPosition = screenPosition - vec{OSWindowRectangle.offset()};

        switch (hitBoxTest(insideWindowPosition).type) {
        case HitBox::Type::BottomResizeBorder: currentCursor = Cursor::None; return HTBOTTOM;
        case HitBox::Type::TopResizeBorder: currentCursor = Cursor::None; return HTTOP;
        case HitBox::Type::LeftResizeBorder: currentCursor = Cursor::None; return HTLEFT;
        case HitBox::Type::RightResizeBorder: currentCursor = Cursor::None; return HTRIGHT;
        case HitBox::Type::BottomLeftResizeCorner: currentCursor = Cursor::None; return HTBOTTOMLEFT;
        case HitBox::Type::BottomRightResizeCorner: currentCursor = Cursor::None; return HTBOTTOMRIGHT;
        case HitBox::Type::TopLeftResizeCorner: currentCursor = Cursor::None; return HTTOPLEFT;
        case HitBox::Type::TopRightResizeCorner: currentCursor = Cursor::None; return HTTOPRIGHT;
        case HitBox::Type::ApplicationIcon: currentCursor = Cursor::None; return HTSYSMENU;
        case HitBox::Type::MoveArea: currentCursor = Cursor::None; return HTCAPTION;
        case HitBox::Type::TextEdit: setCursor(Cursor::TextEdit); return HTCLIENT;
        case HitBox::Type::Button: setCursor(Cursor::Button); return HTCLIENT;
        case HitBox::Type::Default: setCursor(Cursor::Default); return HTCLIENT;
        case HitBox::Type::Outside: currentCursor = Cursor::None; return HTCLIENT;
        default: no_default;
        }
        } break;

    default:
        break;
    }

    // Let DefWindowProc() handle it.
    return -1;
}


}
