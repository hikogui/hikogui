
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
        Window_vulkan_win32::win32WindowClass.hInstance = getShared<Application_win32>()->hInstance;
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
        getShared<Application_win32>()->hInstance, // Instance handle
        this
    );

    if (win32Window == nullptr) {
        BOOST_THROW_EXCEPTION(Application::Error());
    }

    if (!Window_vulkan_win32::firstWindowHasBeenOpened) {
        ShowWindow(win32Window, getShared<Application_win32>()->nCmdShow);
        Window_vulkan_win32::firstWindowHasBeenOpened = true;
    }
    ShowWindow(win32Window, SW_SHOW);

    auto win32SurfaceCreateInfoKHR = vk::Win32SurfaceCreateInfoKHR(
        vk::Win32SurfaceCreateFlagsKHR(),
        getShared<Application_win32>()->hInstance,
        win32Window);

    return getShared<Instance_vulkan>()->intrinsic.createWin32SurfaceKHR(win32SurfaceCreateInfoKHR);

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

Window_vulkan_win32::Window_vulkan_win32(std::shared_ptr<Window::Delegate> delegate, const std::string &title) :
    Window_vulkan(delegate, title, createWindow(title))
{
}

Window_vulkan_win32::~Window_vulkan_win32()
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
    LRESULT result;

    auto win32WindowID = reinterpret_cast<std::uintptr_t>(hwnd);

    if (uMsg == WM_NCCREATE) {
        auto createData = reinterpret_cast<CREATESTRUCT *>(lParam);
        auto window = static_cast<Window_vulkan_win32 *>(createData->lpCreateParams);

        Window_vulkan_win32::win32WindowMap[win32WindowID] = window;
    }

    auto i = Window_vulkan_win32::win32WindowMap.find(win32WindowID);
    if (i != Window_vulkan_win32::win32WindowMap.end()) {
        auto window = i->second;
        result = window->windowProc(hwnd, uMsg, wParam, lParam);

        if (uMsg == WM_DESTROY) {
            Window_vulkan_win32::win32WindowMap.erase(i);
        }
    } else {
        result = DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return result;
}

}}