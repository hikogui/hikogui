// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Audio/globals.hpp"
#include "TTauri/Audio/AudioSystem.hpp"
#include "TTauri/Audio/AudioSystem_win32.hpp"
#include "TTauri/Foundation/globals.hpp"

namespace TTauri::Audio {

AudioGlobals::AudioGlobals(AudioSystemDelegate *audioSystem_delegate) :
    audioSystem_delegate(audioSystem_delegate)
{
    required_assert(Foundation_globals != nullptr);
    required_assert(Audio_globals == nullptr);
    Audio_globals = this;

}

AudioGlobals::~AudioGlobals()
{
    delete _audioSystem;

    required_assert(Audio_globals == this);
    Audio_globals = nullptr;
}

AudioSystem &AudioGlobals::audioSystem() noexcept
{
    if (_audioSystem == nullptr) {
        let lock = std::scoped_lock(mutex);
        if (_audioSystem == nullptr) {
            required_assert(audioSystem_delegate != nullptr);
            _audioSystem = new AudioSystem_win32(audioSystem_delegate);
        }
    }
    return *_audioSystem;

}

}