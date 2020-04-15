// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Audio/globals.hpp"
#include "TTauri/Audio/AudioSystem.hpp"
#include "TTauri/Audio/AudioSystem_win32.hpp"
#include "TTauri/Foundation/globals.hpp"

namespace TTauri::Audio {

void startup()
{
    if (startupCount.fetch_add(1) != 0) {
        // The library has already been initialized.
        return;
    }

    TTauri::startup();
    LOG_AUDIT("TTauri::Audio startup");

    audioSystem = new AudioSystem_win32(audioDelegate);
}

void shutdown()
{
    if (startupCount.fetch_sub(1) != 1) {
        // This is not the last instantiation.
        return;
    }
    LOG_AUDIT("TTauri::Audio shutdown");

    delete audioSystem;

    TTauri::shutdown();
}


}