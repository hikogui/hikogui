// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <coroutine>
#include <type_traits>
#include <exception>

export module hikogui_coroutine_task;
import hikogui_utility;

export namespace hi::inline v1 {

template<typename T>
class task;

template<typename T>
struct task_promise_base {
    T _value;

    void return_void() noexcept
    {
        hi_no_default();
    }

    void return_value(std::convertible_to<T> auto &&value) noexcept
    {
        _value = hi_forward(value);
    }
};

template<>
struct task_promise_base<void> {
    void return_void() noexcept {}
};

template<typename T>
struct task_promise : task_promise_base<T> {
    using value_type = T;
    using handle_type = std::coroutine_handle<task_promise<value_type>>;
    using task_type = task<value_type>;

    static void unhandled_exception()
    {
        throw;
    }

    task<value_type> get_return_object()
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

/** Co-routine task.
 * 
 * @tparam T The type returned by co_return.
 */
template<typename T = void>
class task {
public:
    using value_type = T;
    using promise_type = task_promise<value_type>;
    using handle_type = std::coroutine_handle<promise_type>;

    explicit task(handle_type coroutine) : _coroutine(coroutine) {}

    task() = default;
    ~task()
    {
    }

    task(task const &) = delete;
    task(task &&) = delete;
    task &operator=(task const &) = delete;
    task &operator=(task &&) = delete;

private:
    handle_type _coroutine;
};

} // namespace hi::inline v1
