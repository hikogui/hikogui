// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Audio/globals.hpp"
#include "TTauri/Audio/AudioSystem.hpp"
#include "TTauri/Audio/AudioSystem_win32.hpp"
#include "TTauri/Foundation/globals.hpp"

namespace tt {

/** Reference counter to determine the amount of startup/shutdowns.
*/
static std::atomic<uint64_t> startupCount = 0;

void audio_startup()
{
    if (startupCount.fetch_add(1) != 0) {
        // The library has already been initialized.
        return;
    }

    tt::foundation_startup();
    LOG_INFO("Audio startup");

#if TT_OPERATING_SYSTEM == TT_OS_WINDOWS
    audioSystem = new AudioSystem_win32(audioDelegate);
#else
    audioSystem = nullptr;
#endif
}

void audio_shutdown()
{
    if (startupCount.fetch_sub(1) != 1) {
        // This is not the last instantiation.
        return;
    }
    LOG_INFO("Audio shutdown");

    delete audioSystem;

    tt::foundation_shutdown();
}


}
