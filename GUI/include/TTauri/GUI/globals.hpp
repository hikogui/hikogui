// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/Instance_forward.hpp"
#include "TTauri/GUI/InstanceDelegate.hpp"
#include "TTauri/Foundation/required.hpp"
#include <cstdint>
#include <mutex>
#include <memory>

namespace TTauri::GUI {

struct GUIGlobals;
inline GUIGlobals *GUI_globals = nullptr;

struct GUIGlobals {
private:
    // Cannot use unique_ptr due to Instance not being a complete type.
    Instance *_instance = nullptr;
    InstanceDelegate *instance_delegate = nullptr;

public:
    void *hInstance = nullptr;
    int nCmdShow = 0;

    /*! Global mutex for GUI functionality.
     */
    std::recursive_mutex mutex;


    static constexpr uint32_t defaultNumberOfSwapchainImages = 2;

    GUIGlobals(InstanceDelegate *instance_delegate, void *hInstance, int nCmdShow);
    ~GUIGlobals();
    GUIGlobals(GUIGlobals const &) = delete;
    GUIGlobals &operator=(GUIGlobals const &) = delete;
    GUIGlobals(GUIGlobals &&) = delete;
    GUIGlobals &operator=(GUIGlobals &&) = delete;

    Instance &instance();
};

}

