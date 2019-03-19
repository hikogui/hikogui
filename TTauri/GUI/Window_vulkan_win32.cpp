
#include "Window_vulkan_win32.hpp"

#include "Instance_vulkan.hpp"

#include "TTauri/Application_win32.hpp"

namespace TTauri {
namespace GUI {

using namespace TTauri;


const wchar_t *Window_vulkan_win32::win32WindowClassName = nullptr;
WNDCLASS Window_vulkan_win32::win32WindowClass = {};
bool Window_vulkan_win32::win32WindowClassIsRegistered = false;
std::unordered_map<std::uintptr_t, Window_vulkan_win32 *> Window_vulkan_win32::win32WindowMap = {};
bool Window_vulkan_win32::firstWindowHasBeenOpened = false;


void Window_vulkan_win32::createWindowClass()
{
    if (!Window_vulkan_win32::win32WindowClassIsRegistered) {
         // Register the window class.
        Window_vulkan_win32::win32WindowClassName = L"TTauri Window Class";

        Window_vulkan_win32::win32WindowClass.lpfnWndProc = Window_vulkan_win32::_WindowProc;
        Window_vulkan_win32::win32WindowClass.hInstance = get_singleton<Application_win32>()->hInstance;
        Window_vulkan_win32::win32WindowClass.lpszClassName = Window_vulkan_win32::win32WindowClassName;

        RegisterClass(&win32WindowClass);
    }
    Window_vulkan_win32::win32WindowClassIsRegistered = true;
}

vk::SurfaceKHR Window_vulkan_win32::createWindow(const std::string &title)
{
    Window_vulkan_win32::createWindowClass();

    win32Window = CreateWindowEx(
        0, // Optional window styles.
        Window_vulkan_win32::win32WindowClassName, // Window class
        L"Learn to Program Windows", // Window text
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

    return get_singleton<Instance_vulkan>()->intrinsic.createWin32SurfaceKHR({
        vk::Win32SurfaceCreateFlagsKHR(),
        get_singleton<Application_win32>()->hInstance,
        win32Window
    });

    // XXX Should be done in the loop
    RECT windowRect;
    GetWindowRect(win32Window, &windowRect);

    vk::Rect2D rect;
    rect.offset.x = windowRect.left;
    rect.offset.y = windowRect.top;
    rect.extent.width = windowRect.right - windowRect.left;
    rect.extent.height = windowRect.bottom - windowRect.bottom;
    setWindowRectangle(rect);
}

Window_vulkan_win32::Window_vulkan_win32(const std::shared_ptr<Window::Delegate> &delegate, const std::string &title) :
    Window_vulkan(delegate, title, createWindow(title))
{
}

LRESULT Window_vulkan_win32::windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_DESTROY:
        //PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK Window_vulkan_win32::_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    auto const win32WindowID = reinterpret_cast<std::uintptr_t>(hwnd);

    if (uMsg == WM_NCCREATE) {
        auto const createData = reinterpret_cast<CREATESTRUCT *>(lParam);
        auto const window = static_cast<Window_vulkan_win32 *>(createData->lpCreateParams);

        Window_vulkan_win32::win32WindowMap[win32WindowID] = window;
    }

    auto i = Window_vulkan_win32::win32WindowMap.find(win32WindowID);
    if (i != Window_vulkan_win32::win32WindowMap.end()) {
        auto const window = i->second;
        auto const result = window->windowProc(hwnd, uMsg, wParam, lParam);

        if (uMsg == WM_DESTROY) {
            Window_vulkan_win32::win32WindowMap.erase(i);
        }

        return result;

    } else {
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

}}