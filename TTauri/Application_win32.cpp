// Copyright 2019 Pokitec
// All rights reserved.

#include "Application_win32.hpp"
#include "utils.hpp"
#include "TTauri/GUI/Instance.hpp"
#include "TTauri/GUI/Window.hpp"
#include "strings.hpp"
#include <vulkan/vulkan.hpp>
#include <thread>
#include <string>

namespace TTauri {

Application_win32::Application_win32() :
    mainThreadID(GetCurrentThreadId())
{
    // Resource path, is the same directory as where the executable lives.
    wchar_t modulePathWChar[MAX_PATH];
    if (GetModuleFileNameW(nullptr, modulePathWChar, MAX_PATH) == 0) {
        BOOST_THROW_EXCEPTION(Application_base::ResourceDirError());
    }

    resourceLocation = URL::urlFromWin32Path(modulePathWChar).urlByRemovingFilename();
}

void Application_win32::initialize(const std::shared_ptr<ApplicationDelegate> delegate, HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    this->hInstance = hInstance;
    this->hPrevInstance = hPrevInstance;
    this->pCmdLine = pCmdLine;
    this->nCmdShow = nCmdShow;

    Application_base::initialize(move(delegate));
}

void Application_win32::lastWindowClosed()
{
    runOnMainThread([&]() {
        // Let the application have a change to open new windows from the main thread.
        Application_base::lastWindowClosed();

        if (get_singleton<GUI::Instance>().getNumberOfWindows() == 0) {
            LOG_INFO("Application quiting due to all windows having been closed.");
            PostQuitMessage(0);
        }
    });
}

void Application_win32::runOnMainThread(std::function<void()> function)
{
    let functionP = new std::function<void()>(std::move(function));
    required_assert(functionP);

    auto r = PostThreadMessageW(mainThreadID, WM_APP_CALL_FUNCTION, 0, reinterpret_cast<LPARAM>(functionP));
    required_assert(r != 0);
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
        case WM_APP_CALL_FUNCTION: {
            let functionP = reinterpret_cast<std::function<void()> *>(msg.lParam);
            (*functionP)();
            delete functionP;
            } break;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

}
