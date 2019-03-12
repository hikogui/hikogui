#include "TTauri/Application.hpp"
#include "TTauri/GUI/GUI.hpp"
#include "TTauri/Logging.hpp"
#include <vulkan/vulkan.hpp>
#include <Windows.h>
#include <boost/filesystem.hpp>
#include <memory>
#include <vector>
#include <thread>

using namespace std;
using namespace TTauri;
using namespace TTauri::GUI;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);

}

struct CallbackData {
    shared_ptr<Instance> instance;
};

static void redrawLoop(CallbackData *callbackData)
{
    while (true) {
        callbackData->instance->updateAndRender(0, 0, true);
    }
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    wchar_t _modulePath[MAX_PATH];
    auto r = GetModuleFileNameW(NULL, _modulePath, MAX_PATH);
    if (r == 0 || r == MAX_PATH) {
        LOG_FATAL("Could not retrieve filename of executable.");
        abort();
    }

    auto resourcePath = boost::filesystem::path(_modulePath);
    auto resourceDir = resourcePath.parent_path();
    app = make_shared<Application>(resourceDir);

    auto extensions = vector<const char *>{
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME
    };

    CallbackData callbackData;

    callbackData.instance = make_shared<Instance>(extensions);
    callbackData.instance->setPreferedDeviceUUID({});

    // Register the window class.
    const wchar_t CLASS_NAME[] = L"Sample Window Class";
 
    WNDCLASS wc = { };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    // Create the window.

    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        L"Learn to Program Windows",    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );

    if (hwnd == NULL)
    {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    auto win32SurfaceCreateInfoKHR = vk::Win32SurfaceCreateInfoKHR(
        vk::Win32SurfaceCreateFlagsKHR(),
        hInstance,
        hwnd
    );
    auto surface = callbackData.instance->intrinsic.createWin32SurfaceKHR(win32SurfaceCreateInfoKHR);
    auto window = make_shared<Window>(callbackData.instance.get(), surface);

    RECT windowRect;
    GetWindowRect(hwnd, &windowRect);

    vk::Rect2D rect;
    rect.offset.x = windowRect.left;
    rect.offset.y = windowRect.top;
    rect.extent.width = windowRect.right - windowRect.left;
    rect.extent.height = windowRect.bottom - windowRect.bottom;
    window->setWindowRectangle(rect);

    auto view1 = std::make_shared<ImageView>(app->resourceDir / "lena.png");
    view1->setRectangle({ 100.0, 100.0, 1.0 }, { 200.0, 100.0, 0.0 });
    auto view2 = std::make_shared<ImageView>(app->resourceDir / "lena.png");
    view2->setRectangle({ 200.0, 200.0, 1.0 }, { 200.0, 100.0, 0.0 });
    window->view->add(view1);
    window->view->add(view2);

    if (!callbackData.instance->add(window)) {
        LOG_FATAL("Could not open window.");
        abort();
    }

    // Start update loop.
    thread t = thread(redrawLoop, &callbackData);

    // Run the message loop.
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
