// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <type_traits>
#include <memory>

namespace TTauri {

template<typename T>
inline T &get_singleton() noexcept
{
    static_assert(std::is_default_constructible_v<T>, "Singletons must be default constructible");
    static auto x = std::make_unique<T>();
    return *x;
}

}
