// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/os_detect.hpp"
#include "TTauri/Foundation/assert.hpp"
#include <type_traits>
#include <cstdint>

namespace TTauri {


using namespace std::literals;

template<typename T>
force_inline std::remove_reference_t<T> rvalue_cast(T value)
{
    return value;
}

template <class C>
constexpr auto ssize(const C& c) -> std::common_type_t<std::ptrdiff_t, std::make_signed_t<decltype(c.size())>> 
{
    using R = std::common_type_t<std::ptrdiff_t, std::make_signed_t<decltype(c.size())>>;
    return static_cast<R>(c.size());
}

template <class T, std::ptrdiff_t N>
constexpr std::ptrdiff_t ssize(const T (&array)[N]) noexcept
{
    return N;
}

}