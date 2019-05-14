// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <cstdint>
#include <mutex>

namespace TTauri::GUI {

//! Global TTauri::GUI mutex.
extern std::recursive_mutex mutex;

const uint32_t defaultNumberOfSwapchainImages = 2;

}
