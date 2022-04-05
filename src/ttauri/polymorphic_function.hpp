// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace tt::inline v1 {

template<typename Result, typename... Arguments>
class polymorphic_function_base {
public:
    using result_type = Result;

    virtual ~polymorphic_function_base() = default;

    virtual result_type operator()(Arguments... args) noexcept = 0;
};

template<typename Function, typename T>
class polymorphic_function;

template<typename Result, typename... Arguments, typename Function>
class polymorphic_function<Result(Arguments...), Function> : public polymorphic_function_base<Result, Arguments...> {
public:
    using result_type = Result;

    template<typename Func>
    polymorphic_function(Func &&func) noexcept : _function(std::forward<Func>(func)) {}

    result_type operator()(Arguments... args) noexcept override
    {
        return _function(args...);
    }

private:
    Function _function;
};

template<typename T, typename Func>
polymorphic_function<T>(Func&& func)->polymorphic_function<T, std::decay_t<Func>>;

template<typename Function, typename T>
class polymorphic_async_function;

template<typename Result, typename... Arguments, typename Function>
class polymorphic_async_function<Result(Arguments...), Function> : public polymorphic_function_base<void, Arguments...> {
public:
    using result_type = Result;

    template<typename Func>
    polymorphic_function(Func &&func) noexcept : _function(std::forward<Func>(func)), _promise() {}

    void operator()(Arguments... args) noexcept override
    {
        if constexpr (std::is_same_v<result_type, void>) {
            _function(args...);
            _promise.set_value();
        } else {
            _promise.set_value(_function(args...));
        }
    }

    std::future<result_type> get_future()
    {
        return _promise.get_future();
    }

private:
    Function _function;
    std::promise<result_type> _promise;
};

template<typename T, typename Func>
polymorphic_async_function<T>(Func&& func)->polymorphic_async_function<T, std::decay_t<Func>>;

} // namespace tt::inline v1