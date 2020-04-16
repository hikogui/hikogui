// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <atomic>

namespace TTauri::GUI::Widgets {

/** Reference counter to determine the amount of startup/shutdowns.
*/
inline std::atomic<uint64_t> startupCount = 0;

/** Startup the Text library.
*/
void startup();

/** Shutdown the Text library.
*/
void shutdown();

}