// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/Instance_forward.hpp"
#include "TTauri/GUI/InstanceDelegate.hpp"
#include "TTauri/Required/required.hpp"
#include <cstdint>
#include <mutex>
#include <memory>

#if OPERATING_SYSTEM == OS_WINDOWS
#include <Windows.h>
#endif

namespace TTauri::GUI {

struct GUIGlobals;
inline GUIGlobals *GUI_globals = nullptr;

struct GUIGlobals {
private:
    // Cannot use unique_ptr due to Instance not being a complete type.
    Instance *_instance = nullptr;
    InstanceDelegate *instance_delegate = nullptr;

public:
#if OPERATING_SYSTEM == OS_WINDOWS
    HINSTANCE hInstance = nullptr;
    int nCmdShow = 0;
#endif

    /*! Global mutex for GUI functionality.
     */
    std::recursive_mutex mutex;


    static constexpr uint32_t defaultNumberOfSwapchainImages = 2;

#if OPERATING_SYSTEM == OS_WINDOWS
    GUIGlobals(InstanceDelegate *instance_delegate, HINSTANCE hInstance, int nCmdShow);
#else
    GUIGlobals(InstanceDelegate *instance_delegate);
#endif
    ~GUIGlobals();
    GUIGlobals(GUIGlobals const &) = delete;
    GUIGlobals &operator=(GUIGlobals const &) = delete;
    GUIGlobals(GUIGlobals &&) = delete;
    GUIGlobals &operator=(GUIGlobals &&) = delete;

    Instance &instance();
};

}

