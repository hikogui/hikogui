// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <cstdint>
#include <mutex>
#include <memory>

namespace TTauri::GUI {

//! Global TTauri::GUI mutex.
inline std::recursive_mutex mutex;

inline constexpr uint32_t defaultNumberOfSwapchainImages = 2;

}

