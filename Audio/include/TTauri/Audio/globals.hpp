// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Required/required.hpp"
#include <cstdint>
#include <string>

namespace TTauri::Audio {

struct AudioGlobals;
inline AudioGlobals *Audio_globals = nullptr;

struct AudioGlobals {
private:

public:
    AudioGlobals();
    ~AudioGlobals();
    AudioGlobals(AudioGlobals const &) = delete;
    AudioGlobals &operator=(AudioGlobals const &) = delete;
    AudioGlobals(AudioGlobals &&) = delete;
    AudioGlobals &operator=(AudioGlobals &&) = delete;
};

}