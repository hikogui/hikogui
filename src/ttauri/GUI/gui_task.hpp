// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../required.hpp"
#include <coroutine>
#include <type_traits>

namespace tt::inline v1 {

template<typename T>
class gui_task;

template<typename T>
struct gui_task_promise_base {
    T _value;

    void return_void() noexcept
    {
        tt_no_default();
    }

    void return_value(std::convertible_to<T> auto &&value) noexcept
    {
        _value = tt_forward(value);
    }
};

template<>
struct gui_task_promise_base<void> {
    void return_void() noexcept {}
};

template<typename T>
struct gui_task_promise : gui_task_promise_base<T> {
    using value_type = T;
    using handle_type = std::coroutine_handle<gui_task_promise<value_type>>;
    using task_type = gui_task<value_type>;

    static void unhandled_exception()
    {
        throw;
    }

    gui_task<value_type> get_return_object()
    {
        return task_type{handle_type::from_promise(*this)};
    }

    static std::suspend_never initial_suspend() noexcept
    {
        return {};
    }

    static std::suspend_never final_suspend() noexcept
    {
        return {};
    }
};

template<typename T>
class gui_task {
public:
    using value_type = T;
    using promise_type = gui_task_promise<value_type>;
    using handle_type = std::coroutine_handle<promise_type>;

    explicit gui_task(handle_type coroutine) : _coroutine(coroutine) {}

    gui_task() = default;
    ~gui_task()
    {
        //if (_coroutine) {
        //    _coroutine.destroy();
        //}
    }

    gui_task(gui_task const &) = delete;
    gui_task(gui_task &&) = delete;
    gui_task &operator=(gui_task const &) = delete;
    gui_task &operator=(gui_task &&) = delete;

private:
    handle_type _coroutine;
};

} // namespace tt::inline v1
