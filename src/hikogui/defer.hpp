

#include "../required.hpp"
#include <type_traits>

#pragma once

namespace hi::inline v1 {

template<typename T>
class defer {
public:
    defer() = delete;
    defer(defer &&) = delete;
    defer(defer const &) = delete;
    defer &operator=(defer &&) = delete;
    defer &operator=(defer const &) = delete;

    template<typename Func>
    constexpr defer(Func &&func) noexcept : _func(std::forward<Func>(func)) {}

    constexpr ~defer()
    {
        std::move(_func)();
    }

private:
    T _func;
};

template<typename Func>
defer(Func &&) -> defer<std::remove_cvref_t<Func>>;

}

