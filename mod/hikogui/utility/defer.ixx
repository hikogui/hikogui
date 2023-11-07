// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <type_traits>
#include <functional>

export module hikogui_utility_defer;

export namespace hi { inline namespace v1 {

/** Defer execution of a lambda to the end of the scope.
* 
* c++ guarantees the destruction of local objects in a compound statement (block)
* at the closing brace, in reverse order of declaration. This means that multiple
* `defer` instances will call their lambdas in reverse order of declaration as well.
*/
export class defer {
public:
    defer() = delete;
    defer(defer &&) = delete;
    defer(defer const &) = delete;
    defer &operator=(defer &&) = delete;
    defer &operator=(defer const &) = delete;

    template<std::invocable<> Func>
    [[nodiscard]] defer(Func &&func) noexcept : _func(std::forward<Func>(func)) {}

    ~defer()
    {
        if (_func) {
            _func();
        }
    }

    void cancel() noexcept
    {
        _func = nullptr;
    }

private:
    std::function<void()> _func;
};

}}

