// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

namespace TTauri {

template<typename T, int M>
class fixed {
    T value;

    fixed() = default;
    ~fixed() = default;
    fixed(fixed const &) = default;
    fixed &operator=(fixed const &) = default;
    fixed(fixed &&) = default;
    fixed &operator=(fixed &&) = default;

    template<typename O, std::enabled_if_t<std::is_float_v<O>, int> = 0>  
    explicit constexpr fixed(O &other) noexcept :
        value(static_cast<T>(other * N)) {}

    template<typename O, std::enabled_if_t<std::is_integral_v<O>, int> = 0>
    explicit constexpr fixed(O const &other) noexcept :
        value(static_cast<T>(other) * N) {}

    template<typename O, std::enabled_if_t<std::is_float_v<O>, int> = 0>  
    explicit constexpr fixed &operator=(long double const &other) noexcept {
        value = static_cast<T>(other * N);
        return *this;
    }

    template<typename O, std::enabled_if_t<std::is_integral_v<O>, int> = 0>
    explicit constexpr fixed &operator=(O const &other) noexcept {
        value = static_cast<T>(other) * N;
        return *this;
    }

    template<typename O, std::enabled_if_t<std::is_float_v<O>, int> = 0>  
    explicit operator O () const noexcept {
        return static_cast<O>(value) / N;
    }

    template<typename O, std::enabled_if_t<std::is_integral_v<O>, int> = 0>
    explicit operator O () const noexcept {
        return static_cast<O>(value / N);
    }

}


}

