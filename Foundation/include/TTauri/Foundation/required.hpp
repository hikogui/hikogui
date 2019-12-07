// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/os_detect.hpp"
#include "TTauri/Foundation/assert.hpp"
#include <type_traits>

namespace TTauri {


using namespace std::literals;

template<typename T>
force_inline std::remove_reference_t<T> rvalue_cast(T value) {
    return value;
}

}