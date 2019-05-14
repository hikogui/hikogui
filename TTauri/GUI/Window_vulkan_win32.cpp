// Copyright 2019 Pokitec
// All rights reserved.

#include "Window_vulkan_win32.hpp"
#include "Instance_vulkan_win32.hpp"
#include "TTauri/Application_win32.hpp"
#include "TTauri/strings.hpp"

namespace TTauri::GUI {

using namespace std;
using namespace TTauri;

inline gsl::not_null<void *>to_ptr(LPARAM lParam)
{
    void *ptr;
    memcpy(&ptr, &lParam, sizeof(void *));

    return gsl::not_null<void *>(ptr);
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
        Window_vulkan_win32::win32WindowClass.hInstance = get_singleton<Application_win32>()->hInstance;
        Window_vulkan_win32::win32WindowClass.lpszClassName = Window_vulkan_win32::win32WindowClassName;
        Window_vulkan_win32::win32WindowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
        RegisterClassW(&win32WindowClass);
    }
    Window_vulkan_win32::win32WindowClassIsRegistered = true;
}

vk::SurfaceKHR Window_vulkan_win32::createWindow(const std::string &title)
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
        CW_USEDEFAULT,
        CW_USEDEFAULT,

        NULL, // Parent window
        NULL, // Menu
        get_singleton<Application_win32>()->hInstance, // Instance handle
        this
    );

    if (win32Window == nullptr) {
        BOOST_THROW_EXCEPTION(Application::Error());
    }

    if (!Window_vulkan_win32::firstWindowHasBeenOpened) {
        ShowWindow(win32Window, get_singleton<Application_win32>()->nCmdShow);
        Window_vulkan_win32::firstWindowHasBeenOpened = true;
    }
    ShowWindow(win32Window, SW_SHOW);

    return get_singleton<Instance_vulkan_win32>()->createWin32SurfaceKHR({
        vk::Win32SurfaceCreateFlagsKHR(),
        get_singleton<Application_win32>()->hInstance,
        win32Window
    });
}

Window_vulkan_win32::Window_vulkan_win32(const std::shared_ptr<Window::Delegate> delegate, const std::string title) :
    Window_vulkan(move(delegate), title, createWindow(title))
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
    // Don't lock mutex, the window is about to be destructed.
    // Also no members of this are being accessed.
    auto app = get_singleton<Application_win32>();

    PostThreadMessageW(app->mainThreadID, WM_APP_CLOSING_WINDOW, 0, reinterpret_cast<LPARAM>(this));
}

void Window_vulkan_win32::mainThreadClosingWindow()
{
    // Don't lock mutex, the window is about to be destructed.
    Window_vulkan::closingWindow();
}

void Window_vulkan_win32::openingWindow()
{
    // Don't lock mutex, no members of this are being accessed.
    auto app = get_singleton<Application_win32>();

    PostThreadMessageW(app->mainThreadID, WM_APP_OPENING_WINDOW, 0, reinterpret_cast<LPARAM>(this));
}

void Window_vulkan_win32::mainThreadOpeningWindow()
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    Window_vulkan::openingWindow();
}

LRESULT Window_vulkan_win32::windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // Cannot lock mutex as Window_vulkan_win32 may still be in the process of being constructed.
    switch (uMsg) {
    case WM_MOVING: {
        RECT windowRect;
        memcpy(&windowRect, to_ptr(lParam).get(), sizeof (RECT));

        setWindowPosition(windowRect.left, windowRect.top);
        break;
    }

    case WM_SIZING: {
        RECT windowRect;
        memcpy(&windowRect, to_ptr(lParam).get(), sizeof (RECT));

        setWindowSize(windowRect.right - windowRect.left, windowRect.bottom - windowRect.top);
        break;
    }

    case WM_DESTROY:
        win32Window = nullptr;
        break;

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
            auto const createData = reinterpret_cast<CREATESTRUCT *>(lParam);

            if (createData->lpCreateParams) {
                [[gsl::suppress(lifetime.1)]] {
                    (*Window_vulkan_win32::win32WindowMap)[hwnd] = static_cast<Window_vulkan_win32 *>(createData->lpCreateParams);
                }
            }
        }
    }

    auto i = Window_vulkan_win32::win32WindowMap->find(hwnd);
    if (i != Window_vulkan_win32::win32WindowMap->end()) {
        auto const window = i->second;
        auto const result = window->windowProc(hwnd, uMsg, wParam, lParam);

        if (uMsg == WM_DESTROY) {
            Window_vulkan_win32::win32WindowMap->erase(i);
        }

        return result;
    }

    // Fallback.
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

}
