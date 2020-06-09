// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include <cstdint>
#include <string>
#include <mutex>
#include <atomic>

namespace TTauri {

class AudioSystemDelegate;
class AudioSystem;

inline AudioSystemDelegate *audioDelegate = nullptr;

inline AudioSystem *audioSystem = nullptr;


/** Startup the Text library.
*/
void audio_startup();

/** Shutdown the Text library.
*/
void audio_shutdown();

}