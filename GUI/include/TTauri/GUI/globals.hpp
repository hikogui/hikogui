// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/GUISystem_forward.hpp"
#include "TTauri/GUI/GUISystemDelegate.hpp"
#include "TTauri/GUI/KeyboardBindings.hpp"
#include "TTauri/GUI/ThemeMode.hpp"
#include "TTauri/Foundation/required.hpp"
#include <cstdint>
#include <mutex>
#include <memory>

namespace tt {

/** Delegate for GUI related events.
 */
inline GUISystemDelegate *guiDelegate = nullptr;

/** The GUI system.
 */
inline GUISystem *guiSystem = nullptr;

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

/** Startup the GUI library.
 */
void gui_startup();

/** Shutdown the GUI library.
*/
void gui_shutdown();

}

