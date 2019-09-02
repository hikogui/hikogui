// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include <type_traits>
#include <memory>

namespace TTauri {

template<typename T>
force_inline T &get_singleton() noexcept force_inline_attr
{
    //static_assert(std::is_default_constructible_v<T>, "Singletons must be default constructible");
    static auto x = std::make_unique<T>();
    return *x;
}

}
