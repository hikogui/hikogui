
#include "Window_win32.hpp"

#include "Instance.hpp"

#include "TTauri/Application_win32.hpp"

namespace TTauri {
namespace GUI {

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result;

    auto win32WindowID = reinterpret_cast<std::uintptr_t>(hwnd);

    if (uMsg == WM_NCCREATE) {
        auto createData = reinterpret_cast<CREATESTRUCT *>(lParam);
        auto window = reinterpret_cast<Window_win32 *>(createData->lpCreateParams);

        Window_win32::win32WindowMap[win32WindowID] = window;
    }

    auto i = Window_win32::win32WindowMap.find(win32WindowID);
    if (i != Window_win32::win32WindowMap.end()) {
        auto window = i->second;
        result = window->windowProc(hwnd, uMsg, wParam, lParam);

        if (uMsg == WM_DESTROY) {
            Window_win32::win32WindowMap.erase(i);
        }
    } else {
        result = DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return result;
}

void Window_win32::createWindowClass()
{
    if (!Window_win32::win32WindowClassIsRegistered) {
        auto win32Instance = std::dynamic_pointer_cast<Application_win32>(Application::shared)->win32Instance;

        // Register the window class.
        Window_win32::win32WindowClassName = L"TTauri Window Class";

        Window_win32::win32WindowClass.lpfnWndProc = WindowProc;
        Window_win32::win32WindowClass.hInstance = win32Instance;
        Window_win32::win32WindowClass.lpszClassName = Window_win32::win32WindowClassName;

        RegisterClass(&win32WindowClass);
    }
    Window_win32::win32WindowClassIsRegistered = true;
}

vk::SurfaceKHR Window_win32::createWindow(Instance *instance, const std::string &title, int win32Show)
{
    Window_win32::createWindowClass();

    auto win32Instance = std::dynamic_pointer_cast<Application_win32>(Application::shared)->win32Instance;
    win32Window = CreateWindowEx(
        0, // Optional window styles.
        Window_win32::win32WindowClassName, // Window class
        L"Learn to Program Windows", // Window text
        WS_OVERLAPPEDWINDOW, // Window style

        // Size and position
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,

        NULL, // Parent window
        NULL, // Menu
        win32Instance, // Instance handle
        this
    );

    if (win32Window == NULL) {
        BOOST_THROW_EXCEPTION(Application::Error());
    }

    if (!Window_win32::firstWindowHasBeenOpened) {
        ShowWindow(win32Window, win32Show);
        Window_win32::firstWindowHasBeenOpened = true;
    }
    ShowWindow(win32Window, SW_SHOW);

    auto win32SurfaceCreateInfoKHR = vk::Win32SurfaceCreateInfoKHR(
        vk::Win32SurfaceCreateFlagsKHR(),
        win32Instance,
        win32Window);

    return instance->intrinsic.createWin32SurfaceKHR(win32SurfaceCreateInfoKHR);

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

Window_win32::Window_win32(Instance *instance, std::shared_ptr<Window::Delegate> delegate, const std::string &title, int win32Show) :
    Window(instance, delegate, title, createWindow(instance, title, win32Show))
{
}

Window_win32::~Window_win32()
{
}

LRESULT Window_win32::windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

const wchar_t *Window_win32::win32WindowClassName = nullptr;
WNDCLASS Window_win32::win32WindowClass = {};
bool Window_win32::win32WindowClassIsRegistered = false;
std::unordered_map<std::uintptr_t, Window_win32 *> Window_win32::win32WindowMap = {};
bool Window_win32::firstWindowHasBeenOpened = false;

}}