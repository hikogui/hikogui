// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/Instance_forward.hpp"
#include "TTauri/GUI/InstanceDelegate.hpp"
#include "TTauri/GUI/KeyboardBindings.hpp"
#include "TTauri/Foundation/required.hpp"
#include <cstdint>
#include <mutex>
#include <memory>

namespace TTauri::GUI {

/** Delegate for GUI related events.
 */
inline InstanceDelegate *guiDelegate = nullptr;

inline Instance *guiSystem = nullptr;

/** Windows GUI-application instance handle.
 */
inline void *hInstance = nullptr;

/** Windows GUI-application startup command.
 */
inline int nCmdShow = 0;

/*! Global mutex for GUI functionality.
*/
inline std::recursive_mutex guiMutex;

/** Global keyboard bindings.
*/
inline KeyboardBindings keyboardBindings;

inline constexpr uint32_t defaultNumberOfSwapchainImages = 2;

/** Reference counter to determine the amount of startup/shutdowns.
 */
inline std::atomic<uint64_t> startupCount = 0;

/** Startup the GUI library.
 */
void startup();

/** Shutdown the GUI library.
*/
void shutdown();

}

