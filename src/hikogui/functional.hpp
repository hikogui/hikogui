// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "utility/module.hpp"
#include <type_traits>
#include <future>

namespace hi::inline v1 {

template<typename Proto>
class function;

template<typename Result, typename... Arguments>
class function<Result(Arguments...)> {
public:
    using result_type = Result;

    virtual ~function() = default;
    virtual result_type operator()(Arguments... arguments) = 0;
};

namespace detail {

template<typename Function, typename Proto>
class function_impl;

template<typename Function, typename Result, typename... Arguments>
class function_impl<Function, Result(Arguments...)> final : public function<Result(Arguments...)> {
public:
    using result_type = typename function<Result(Arguments...)>::result_type;

    template<typename Func>
    function_impl(Func&& func) noexcept : _function(std::forward<Func>(func))
    {
    }

    result_type operator()(Arguments... arguments) override
    {
        return _function(std::forward<decltype(arguments)>(arguments)...);
    }

private:
    Function _function;
};

template<typename Function, typename Proto>
class async_function_impl;

template<typename Function, typename... Arguments>
class async_function_impl<Function, void(Arguments...)> final : public function<void(Arguments...)> {
public:
    using result_type = void;
    using async_result_type = std::invoke_result_t<Function, Arguments...>;

    template<typename Func>
    async_function_impl(Func&& func) noexcept : _function(std::forward<Func>(func)), _promise()
    {
    }

    void operator()(Arguments... arguments) override
    {
        if constexpr (std::is_same_v<async_result_type, void>) {
            try {
                _function(std::forward<decltype(arguments)>(arguments)...);
                _promise.set_value();
            } catch (...) {
                _promise.set_exception(std::current_exception());
            }
        } else {
            try {
                _promise.set_value(_function(std::forward<decltype(arguments)>(arguments)...));
            } catch (...) {
                _promise.set_exception(std::current_exception());
            }
        }
    }

    [[nodiscard]] std::future<async_result_type> get_future() noexcept
    {
        return _promise.get_future();
    }

private:
    Function _function;
    std::promise<async_result_type> _promise;
};

}

template<typename Proto, typename Func>
auto make_function(Func&& func)
{
    return detail::function_impl<std::decay_t<Func>, Proto>{std::forward<Func>(func)};
}

template<typename Proto, typename Func>
auto make_async_function(Func&& func)
{
    return detail::async_function_impl<std::decay_t<Func>, Proto>{std::forward<Func>(func)};
}

} // namespace hi::inline v1
