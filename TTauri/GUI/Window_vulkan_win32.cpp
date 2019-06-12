// Copyright 2019 Pokitec
// All rights reserved.

#include "Window_vulkan_win32.hpp"
#include "Instance.hpp"
#include "TTauri/all.hpp"
#include <windowsx.h>

namespace TTauri::GUI {

using namespace std;
using namespace TTauri;

template<typename T>
inline gsl::not_null<T *>to_ptr(LPARAM lParam)
{
    T *ptr;
    memcpy(&ptr, &lParam, sizeof(T *));

    return gsl::not_null<T *>(ptr);
}

const wchar_t *Window_vulkan_win32::win32WindowClassName = nullptr;
WNDCLASSW Window_vulkan_win32::win32WindowClass = {};
bool Window_vulkan_win32::win32WindowClassIsRegistered = false;
std::shared_ptr<std::unordered_map<HWND, Window_vulkan_win32 *>> Window_vulkan_win32::win32WindowMap = {};
bool Window_vulkan_win32::firstWindowHasBeenOpened = false;

void Window_vulkan_win32::createWindowClass()
{
    if (!Window_vulkan_win32::win32WindowClassIsRegistered) {
         // Register the window class.
        Window_vulkan_win32::win32WindowClassName = L"TTauri Window Class";

        Window_vulkan_win32::win32WindowClass.lpfnWndProc = Window_vulkan_win32::_WindowProc;
        Window_vulkan_win32::win32WindowClass.hInstance = application->hInstance;
        Window_vulkan_win32::win32WindowClass.lpszClassName = Window_vulkan_win32::win32WindowClassName;
        Window_vulkan_win32::win32WindowClass.hCursor = nullptr;
        RegisterClassW(&win32WindowClass);
    }
    Window_vulkan_win32::win32WindowClassIsRegistered = true;
}

void Window_vulkan_win32::createWindow(const std::string &title, u32extent2 extent)
{
    Window_vulkan_win32::createWindowClass();

    auto u16title = translateString<wstring>(title);

    win32Window = CreateWindowExW(
        0, // Optional window styles.
        Window_vulkan_win32::win32WindowClassName, // Window class
        u16title.data(), // Window text
        WS_OVERLAPPEDWINDOW, // Window style

        // Size and position
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        extent.width(),
        extent.height(),

        NULL, // Parent window
        NULL, // Menu
        application->hInstance, // Instance handle
        this
    );

    if (win32Window == nullptr) {
        BOOST_THROW_EXCEPTION(Application::Error());
    }

    if (!Window_vulkan_win32::firstWindowHasBeenOpened) {
        ShowWindow(win32Window, application->nCmdShow);
        Window_vulkan_win32::firstWindowHasBeenOpened = true;
    }

    trackMouseLeaveEventParameters.cbSize = sizeof(trackMouseLeaveEventParameters);
    trackMouseLeaveEventParameters.dwFlags = TME_LEAVE;
    trackMouseLeaveEventParameters.hwndTrack = win32Window;
    trackMouseLeaveEventParameters.dwHoverTime = HOVER_DEFAULT;

    ShowWindow(win32Window, SW_SHOW);
}

Window_vulkan_win32::Window_vulkan_win32(const std::shared_ptr<WindowDelegate> delegate, const std::string title) :
    Window_vulkan(move(delegate), title), trackMouseLeaveEventParameters()
{
}

Window_vulkan_win32::~Window_vulkan_win32()
{
    try {
        [[gsl::suppress(f.6)]] {
            if (win32Window != nullptr) {
                LOG_FATAL("win32Window was not destroyed before Window '%s' was destructed.") % title;
                abort();
            }
        }
    } catch (...) {
        abort();
    }
}

void Window_vulkan_win32::closingWindow()
{
    // Don't lock mutex, no members of this are being accessed.
    PostThreadMessageW(application->mainThreadID, WM_APP_CLOSING_WINDOW, 0, reinterpret_cast<LPARAM>(this));
}

void Window_vulkan_win32::mainThreadClosingWindow()
{
    // Don't lock mutex, the window is about to be destructed.
    Window_vulkan::closingWindow();
}

void Window_vulkan_win32::openingWindow()
{
    // Don't lock mutex, no members of this are being accessed.
    PostThreadMessageW(application->mainThreadID, WM_APP_OPENING_WINDOW, 0, reinterpret_cast<LPARAM>(this));
}

void Window_vulkan_win32::mainThreadOpeningWindow()
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    Window_vulkan::openingWindow();

    // Delegate has been called, layout of widgets has been calculated for the
    // minimum and maximum size of the window.
    u32extent2 windowExtent = minimumWindowExtent;
    createWindow(title, windowExtent);
}

vk::SurfaceKHR Window_vulkan_win32::getSurface()
{
    return instance->createWin32SurfaceKHR({
        vk::Win32SurfaceCreateFlagsKHR(),
        application->hInstance,
        win32Window
    });
}


LRESULT Window_vulkan_win32::windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    {
        std::scoped_lock lock(TTauri::GUI::mutex);

        RECT* rect;
        MINMAXINFO* minmaxinfo;

        MouseEvent mouseEvent;

        switch (uMsg) {    
        case WM_DESTROY:
            win32Window = nullptr;
            state = State::WINDOW_LOST;
            break;

        case WM_SIZING:
            rect = to_ptr<RECT>(lParam);
            OSWindowRectangle.offset.x = rect->left;
            OSWindowRectangle.offset.y = 0; // XXX Without screen height, it is not possible to calculate the y of the left-bottom corner.
            // XXX - figure out size of decoration to remove these constants.
            OSWindowRectangle.extent.x = (rect->right - rect->left) - 26;
            OSWindowRectangle.extent.y = (rect->bottom - rect->top) - 39;
            break;

        case WM_ENTERSIZEMOVE:
            resizing = true;
            break;

        case WM_EXITSIZEMOVE:
            resizing = false;
            break;
    
        case WM_GETMINMAXINFO:
            minmaxinfo = to_ptr<MINMAXINFO>(lParam);
            // XXX - figure out size of decoration to remove these constants.
            minmaxinfo->ptMaxSize.x = boost::numeric_cast<long>(maximumWindowExtent.width() + 26);
            minmaxinfo->ptMaxSize.y = boost::numeric_cast<long>(maximumWindowExtent.height() + 39);
            minmaxinfo->ptMinTrackSize.x = boost::numeric_cast<long>(minimumWindowExtent.width() + 26);
            minmaxinfo->ptMinTrackSize.y = boost::numeric_cast<long>(minimumWindowExtent.height() + 39);
            minmaxinfo->ptMaxTrackSize.x = boost::numeric_cast<long>(maximumWindowExtent.width() + 26);
            minmaxinfo->ptMaxTrackSize.y = boost::numeric_cast<long>(maximumWindowExtent.height() + 39);
            break;

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
                    LOG_ERROR("Could not track leave event '%s'") % getLastErrorMessage();
                }
                trackingMouseLeaveEvent = true;
            }
            mouseEvent.type = MouseEvent::Type::Move;
            goto parseMouseEvent;

        parseMouseEvent:
            mouseEvent.position.x = boost::numeric_cast<float>(GET_X_LPARAM(lParam));
            mouseEvent.position.y = boost::numeric_cast<float>(currentWindowExtent.height() - GET_Y_LPARAM(lParam));
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

        default:
            break;
        }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK Window_vulkan_win32::_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (!win32WindowMap) {
        win32WindowMap = make_shared<std::unordered_map<HWND, Window_vulkan_win32 *>>();
    }

    if (uMsg == WM_NCCREATE && lParam) {
        [[gsl::suppress(type.1)]] {
            let createData = reinterpret_cast<CREATESTRUCT *>(lParam);

            if (createData->lpCreateParams) {
                [[gsl::suppress(lifetime.1)]] {
                    (*Window_vulkan_win32::win32WindowMap)[hwnd] = static_cast<Window_vulkan_win32 *>(createData->lpCreateParams);
                }
            }
        }
    }

    auto i = Window_vulkan_win32::win32WindowMap->find(hwnd);
    if (i != Window_vulkan_win32::win32WindowMap->end()) {
        let window = i->second;
        let result = window->windowProc(hwnd, uMsg, wParam, lParam);

        if (uMsg == WM_DESTROY) {
            Window_vulkan_win32::win32WindowMap->erase(i);
        }

        return result;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

}
