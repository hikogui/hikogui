// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Required/required.hpp"
#include <cstdint>
#include <string>
#include <mutex>

namespace TTauri::Audio {
class AudioSystem;
class AudioSystemDelegate;

struct AudioGlobals;
inline AudioGlobals *Audio_globals = nullptr;

struct AudioGlobals {
private:
    AudioSystem *_audioSystem = nullptr;
    AudioSystemDelegate *audioSystem_delegate = nullptr;

public:
    /*! Global mutex for Audio functionality.
    */
    std::recursive_mutex mutex;

    AudioGlobals(AudioSystemDelegate *audioSystem_delegate);
    ~AudioGlobals();
    AudioGlobals(AudioGlobals const &) = delete;
    AudioGlobals &operator=(AudioGlobals const &) = delete;
    AudioGlobals(AudioGlobals &&) = delete;
    AudioGlobals &operator=(AudioGlobals &&) = delete;

    AudioSystem &audioSystem() noexcept;
};

}