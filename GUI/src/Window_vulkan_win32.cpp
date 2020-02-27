// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/GUI/Window_vulkan_win32.hpp"
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

        win32WindowClass.lpfnWndProc = _WindowProc;
        win32WindowClass.hInstance = reinterpret_cast<HINSTANCE>(GUI_globals->hInstance);
        win32WindowClass.lpszClassName = win32WindowClassName;
        win32WindowClass.hCursor = nullptr;
        RegisterClassW(&win32WindowClass);
    }
    win32WindowClassIsRegistered = true;
}

void Window_vulkan_win32::createWindow(const std::string &title, extent2 extent)
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
        numeric_cast<int>(extent.width()),
        numeric_cast<int>(extent.height()),

        NULL, // Parent window
        NULL, // Menu
        reinterpret_cast<HINSTANCE>(GUI_globals->hInstance), // Instance handle
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
        ShowWindow(reinterpret_cast<HWND>(win32Window), GUI_globals->nCmdShow);
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
        std::scoped_lock lock(GUI_globals->mutex);

        Window_vulkan::openingWindow();

        // Delegate has been called, layout of widgets has been calculated for the
        // minimum and maximum size of the window.
        extent2 windowExtent = minimumWindowExtent;
        createWindow(title, windowExtent);
    });
}

vk::SurfaceKHR Window_vulkan_win32::getSurface() const
{
    return GUI_globals->instance().createWin32SurfaceKHR({
        vk::Win32SurfaceCreateFlagsKHR(),
        reinterpret_cast<HINSTANCE>(GUI_globals->hInstance),
        reinterpret_cast<HWND>(win32Window)
    });
}

void Window_vulkan_win32::setOSWindowRectangleFromRECT(RECT rect) noexcept
{
    // XXX Without screen height, it is not possible to calculate the y of the left-bottom corner.
    OSWindowRectangle.offset.x = rect.left;
    OSWindowRectangle.offset.y = 0 - rect.bottom;
    OSWindowRectangle.extent.width() = (rect.right - rect.left);
    OSWindowRectangle.extent.height() = (rect.bottom - rect.top);
    setModified();
}

void Window_vulkan_win32::setCursor(Cursor cursor) noexcept {
    if (cursor == currentCursor) {
        return;
    }
    currentCursor = cursor;

    switch (cursor) {
    case Cursor::None:
        SetCursor(LoadCursorW(nullptr, IDC_APPSTARTING));
        break;
    case Cursor::Default:
        SetCursor(LoadCursorW(nullptr, IDC_ARROW));
        break;
    case Cursor::Clickable:
        SetCursor(LoadCursorW(nullptr, IDC_HAND));
        break;
    default:
        SetCursor(LoadCursorW(nullptr, IDC_NO));
        break;
    }
}

[[nodiscard]] KeyboardModifiers Window_vulkan_win32::getKeyboardModifiers() noexcept
{
    auto r = KeyboardModifiers::Idle;

    if (GetKeyState(VK_SHIFT) != 0) {
        r |= KeyboardModifiers::Shift;
    }
    if (GetKeyState(VK_CONTROL) != 0) {
        r |= KeyboardModifiers::Control;
    }
    if (GetKeyState(VK_MENU) != 0) {
        r |= KeyboardModifiers::Alt;
    }
    if (GetKeyState(VK_LWIN) != 0 || GetKeyState(VK_RWIN) != 0) {
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

void Window_vulkan_win32::handle_virtual_key_code(int key_code) noexcept
{
    LOG_ERROR("Key 0x{:x} down={}", key_code);

    auto state = getKeyboardState();
    auto modifiers = getKeyboardModifiers();

    switch (key_code) {
    case VK_SNAPSHOT: [[fallthrough]];
    case VK_PRINT: return handleKeyboardEvent(state, modifiers, CommandKey::Print);
    case VK_HOME: return handleKeyboardEvent(state, modifiers, CommandKey::Home);
    case VK_END: return handleKeyboardEvent(state, modifiers, CommandKey::End);
    case VK_LEFT: return handleKeyboardEvent(state, modifiers, CommandKey::LeftArrow);
    case VK_RIGHT: return handleKeyboardEvent(state, modifiers, CommandKey::RightArrow);
    case VK_UP: return handleKeyboardEvent(state, modifiers, CommandKey::UpArrow);
    case VK_DOWN: return handleKeyboardEvent(state, modifiers, CommandKey::DownArrow);
    case VK_BACK: return handleKeyboardEvent(state, modifiers, CommandKey::Backspace);
    case VK_TAB: return handleKeyboardEvent(state, modifiers, CommandKey::Tab);
    case VK_RETURN: return handleKeyboardEvent(state, modifiers, CommandKey::Enter);
    case VK_F1: return handleKeyboardEvent(state, modifiers, CommandKey::F1);
    case VK_F2: return handleKeyboardEvent(state, modifiers, CommandKey::F2);
    case VK_F3: return handleKeyboardEvent(state, modifiers, CommandKey::F3);
    case VK_F4: return handleKeyboardEvent(state, modifiers, CommandKey::F4);
    case VK_F5: return handleKeyboardEvent(state, modifiers, CommandKey::F5);
    case VK_F6: return handleKeyboardEvent(state, modifiers, CommandKey::F6);
    case VK_F7: return handleKeyboardEvent(state, modifiers, CommandKey::F7);
    case VK_F8: return handleKeyboardEvent(state, modifiers, CommandKey::F8);
    case VK_F9: return handleKeyboardEvent(state, modifiers, CommandKey::F9);
    case VK_F10: return handleKeyboardEvent(state, modifiers, CommandKey::F10);
    case VK_F11: return handleKeyboardEvent(state, modifiers, CommandKey::F11);
    case VK_F12: return handleKeyboardEvent(state, modifiers, CommandKey::F12);
    case VK_CLEAR: return handleKeyboardEvent(state, modifiers, CommandKey::Clear);
    case VK_PAUSE: return handleKeyboardEvent(state, modifiers, CommandKey::PauseBreak);
    case VK_VOLUME_MUTE: return handleKeyboardEvent(state, modifiers, CommandKey::VolumeMute);
    case VK_INSERT: return handleKeyboardEvent(state, modifiers, CommandKey::Insert);
    case VK_ESCAPE: return handleKeyboardEvent(state, modifiers, CommandKey::Escape);
    case VK_PRIOR: return handleKeyboardEvent(state, modifiers, CommandKey::PageUp);
    case VK_VOLUME_UP: return handleKeyboardEvent(state, modifiers, CommandKey::VolumeUp);
    case VK_VOLUME_DOWN: return handleKeyboardEvent(state, modifiers, CommandKey::VolumeDown);
    case VK_DELETE: return handleKeyboardEvent(state, modifiers, CommandKey::Delete);
    case VK_OEM_PLUS: return handleKeyboardEvent(state, modifiers, '+');
    case VK_OEM_COMMA: return handleKeyboardEvent(state, modifiers, ',');
    case VK_OEM_MINUS: return handleKeyboardEvent(state, modifiers, '-');
    case VK_OEM_PERIOD: return handleKeyboardEvent(state, modifiers, '.');
    default:
        if (key_code >= 'A' && key_code <= 'Z') {
            return handleKeyboardEvent(state, modifiers, key_code);
        } else if (key_code >= '0' && key_code <= '9') {
            return handleKeyboardEvent(state, modifiers, key_code);
        }
    }
}

int Window_vulkan_win32::windowProc(unsigned int uMsg, uint64_t wParam, int64_t lParam)
{
    std::scoped_lock lock(GUI_globals->mutex);

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
        setModified();
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
        setModified();
        break;

    case WM_GETMINMAXINFO: {
        let minmaxinfo = to_ptr<MINMAXINFO>(lParam);
        // XXX - figure out size of decoration to remove these constants.
        minmaxinfo->ptMaxSize.x = numeric_cast<long>(maximumWindowExtent.width());
        minmaxinfo->ptMaxSize.y = numeric_cast<long>(maximumWindowExtent.height());
        minmaxinfo->ptMinTrackSize.x = numeric_cast<long>(minimumWindowExtent.width());
        minmaxinfo->ptMinTrackSize.y = numeric_cast<long>(minimumWindowExtent.height());
        minmaxinfo->ptMaxTrackSize.x = numeric_cast<long>(maximumWindowExtent.width());
        minmaxinfo->ptMaxTrackSize.y = numeric_cast<long>(maximumWindowExtent.height());
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

    case WM_KEYDOWN: {
        bool extended = (numeric_cast<uint32_t>(lParam) & 0x01000000) != 0;
        handle_virtual_key_code(numeric_cast<int>(wParam));
        } break;

    case WM_LBUTTONDOWN:
        mouseEvent.type = MouseEvent::Type::ButtonDown;
        mouseEvent.cause.leftButton = true;
        goto parseMouseEvent;
    case WM_LBUTTONUP:
        mouseEvent.type = MouseEvent::Type::ButtonUp;
        mouseEvent.cause.leftButton = true;
        goto parseMouseEvent;
    case WM_LBUTTONDBLCLK:
        mouseEvent.type = MouseEvent::Type::ButtonDoubleClick;
        mouseEvent.cause.leftButton = true;
        goto parseMouseEvent;
    case WM_MBUTTONDOWN:
        mouseEvent.type = MouseEvent::Type::ButtonDown;
        mouseEvent.cause.middleButton = true;
        goto parseMouseEvent;
    case WM_MBUTTONUP:
        mouseEvent.type = MouseEvent::Type::ButtonUp;
        mouseEvent.cause.middleButton = true;
        goto parseMouseEvent;
    case WM_MBUTTONDBLCLK:
        mouseEvent.type = MouseEvent::Type::ButtonDoubleClick;
        mouseEvent.cause.middleButton = true;
        goto parseMouseEvent;
    case WM_RBUTTONDOWN:
        mouseEvent.type = MouseEvent::Type::ButtonDown;
        mouseEvent.cause.rightButton = true;
        goto parseMouseEvent;
    case WM_RBUTTONUP:
        mouseEvent.type = MouseEvent::Type::ButtonUp;
        mouseEvent.cause.rightButton = true;
        goto parseMouseEvent;
    case WM_RBUTTONDBLCLK:
        mouseEvent.type = MouseEvent::Type::ButtonDoubleClick;
        mouseEvent.cause.rightButton = true;
        goto parseMouseEvent;
    case WM_XBUTTONDOWN:
        mouseEvent.type = MouseEvent::Type::ButtonDown;
        mouseEvent.cause.x1Button = (GET_XBUTTON_WPARAM(wParam) & XBUTTON1) > 0;
        mouseEvent.cause.x2Button = (GET_XBUTTON_WPARAM(wParam) & XBUTTON2) > 0;
        goto parseMouseEvent;
    case WM_XBUTTONUP:
        mouseEvent.type = MouseEvent::Type::ButtonUp;
        mouseEvent.cause.x1Button = (GET_XBUTTON_WPARAM(wParam) & XBUTTON1) > 0;
        mouseEvent.cause.x2Button = (GET_XBUTTON_WPARAM(wParam) & XBUTTON2) > 0;
        goto parseMouseEvent;
    case WM_XBUTTONDBLCLK:
        mouseEvent.type = MouseEvent::Type::ButtonDoubleClick;
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
        mouseEvent.position.x = numeric_cast<float>(GET_X_LPARAM(lParam));
        mouseEvent.position.y = numeric_cast<float>(currentWindowExtent.height() - GET_Y_LPARAM(lParam));
        mouseEvent.down.controlKey = (GET_KEYSTATE_WPARAM(wParam) & MK_CONTROL) > 0;
        mouseEvent.down.leftButton = (GET_KEYSTATE_WPARAM(wParam) & MK_LBUTTON) > 0;
        mouseEvent.down.middleButton = (GET_KEYSTATE_WPARAM(wParam) & MK_MBUTTON) > 0;
        mouseEvent.down.rightButton = (GET_KEYSTATE_WPARAM(wParam) & MK_RBUTTON) > 0;
        mouseEvent.down.shiftKey = (GET_KEYSTATE_WPARAM(wParam) & MK_SHIFT) > 0;
        mouseEvent.down.x1Button = (GET_KEYSTATE_WPARAM(wParam) & MK_XBUTTON1) > 0;
        mouseEvent.down.x2Button = (GET_KEYSTATE_WPARAM(wParam) & MK_XBUTTON2) > 0;
        handleMouseEvent(mouseEvent);
        break;

    case WM_MOUSELEAVE:
        // After this event we need to ask win32 to track the mouse again.
        trackingMouseLeaveEvent = false;

        // Force currentCursor to None so that the Window is in a fresh
        // state when the mouse reenters it.
        currentCursor = Cursor::None;

        handleMouseEvent(ExitedMouseEvent());
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
        let screenPosition = glm::vec2{
            numeric_cast<float>(GET_X_LPARAM(lParam)),
            0.0 - numeric_cast<float>(GET_Y_LPARAM(lParam))
        };

        let insideWindowPosition = screenPosition - glm::vec2(OSWindowRectangle.offset);

        switch (hitBoxTest(insideWindowPosition)) {
        case HitBox::BottomResizeBorder: return HTBOTTOM;
        case HitBox::TopResizeBorder: return HTTOP;
        case HitBox::LeftResizeBorder: return HTLEFT;
        case HitBox::RightResizeBorder: return HTRIGHT;
        case HitBox::BottomLeftResizeCorner: return HTBOTTOMLEFT;
        case HitBox::BottomRightResizeCorner: return HTBOTTOMRIGHT;
        case HitBox::TopLeftResizeCorner: return HTTOPLEFT;
        case HitBox::TopRightResizeCorner: return HTTOPRIGHT;
        case HitBox::ApplicationIcon: return HTSYSMENU;
        case HitBox::MoveArea: return HTCAPTION;
        case HitBox::NoWhereInteresting: currentCursor = Cursor::None; return HTCLIENT;
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
