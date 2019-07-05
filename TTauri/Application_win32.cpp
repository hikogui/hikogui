// Copyright 2019 Pokitec
// All rights reserved.

#include "Application_win32.hpp"
#include "utils.hpp"
#include "TTauri/GUI/Instance.hpp"
#include "TTauri/GUI/Window.hpp"
#include <vulkan/vulkan.hpp>
#include <thread>

namespace TTauri {

void Application_win32::initialize(const std::shared_ptr<ApplicationDelegate> delegate, HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    this->hInstance = hInstance;
    this->hPrevInstance = hPrevInstance;
    this->pCmdLine = pCmdLine;
    this->nCmdShow = nCmdShow;

    // Resource path, is the same directory as where the executable lives.
    wchar_t modulePathWChar[MAX_PATH];
    if (GetModuleFileNameW(nullptr, modulePathWChar, MAX_PATH) == 0) {
        BOOST_THROW_EXCEPTION(Application_base::ResourceDirError());
    }

    let modulePath = std::filesystem::path(modulePathWChar);

    resourceDir = modulePath.parent_path();

    Application_base::initialize(move(delegate));
}

void Application_win32::lastWindowClosed()
{
    PostThreadMessageW(mainThreadID, WM_APP_LAST_WINDOW_CLOSED, 0, 0);
}

void Application_win32::mainThreadLastWindowClosed()
{
    // Let the application have a change to open new windows from the main thread.
    Application_base::lastWindowClosed();

    if (singleton<GUI::Instance>->getNumberOfWindows() == 0) {
        LOG_INFO("Application quiting due to all windows having been closed.");
        PostQuitMessage(0);
    }
}

void Application_win32::startingLoop()
{
    Application_base::startingLoop();
}

int Application_win32::loop()
{
    startingLoop();

    // Run the message loop.
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        switch (msg.message) {
        case WM_APP_LAST_WINDOW_CLOSED:
            mainThreadLastWindowClosed();
            break;

        case WM_APP_CLOSING_WINDOW:
            reinterpret_cast<GUI::Window *>(msg.lParam)->mainThreadClosingWindow();
            break;

        case WM_APP_OPENING_WINDOW:
            reinterpret_cast<GUI::Window *>(msg.lParam)->mainThreadOpeningWindow();
            break;

        case WM_APP_CLOSE_WINDOW:
            reinterpret_cast<GUI::Window *>(msg.lParam)->mainThreadCloseWindow();
            break;

        case WM_APP_MINIMIZE_WINDOW:
            reinterpret_cast<GUI::Window *>(msg.lParam)->mainThreadMinimizeWindow();
            break;

        case WM_APP_MAXIMIZE_WINDOW:
            reinterpret_cast<GUI::Window *>(msg.lParam)->mainThreadMaximizeWindow();
            break;

        case WM_APP_NORMALIZE_WINDOW:
            reinterpret_cast<GUI::Window *>(msg.lParam)->mainThreadNormalizeWindow();
            break;

        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

}
