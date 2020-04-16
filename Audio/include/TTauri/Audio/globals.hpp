// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include <cstdint>
#include <string>
#include <mutex>
#include <atomic>

namespace TTauri::Audio {

class AudioSystemDelegate;
class AudioSystem;

inline AudioSystemDelegate *audioDelegate = nullptr;

inline AudioSystem *audioSystem = nullptr;

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