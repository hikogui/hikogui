// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "awaitable.hpp"
#include "../concurrency/concurrency.hpp"
#include "../time/time.hpp"
#include "../macros.hpp"
#include <chrono>
#include <coroutine>
#include <functional>
#include <stop_token>

hi_export_module(hikogui.dispatch : awaitable_stop_token_intf);

hi_export namespace hi::inline v1 {
class loop;

class awaitable_stop_token {
public:
    awaitable_stop_token(awaitable_stop_token &&) noexcept = default;
    awaitable_stop_token &operator=(awaitable_stop_token &&) noexcept = default;

    awaitable_stop_token(awaitable_stop_token const &other) noexcept : _stop_token(other._stop_token), _stop_callback_ptr() {}

    awaitable_stop_token &operator=(awaitable_stop_token const &other) noexcept
    {
        _stop_callback_ptr = nullptr;
        _stop_token = other._stop_token;
        return *this;
    }

    awaitable_stop_token(std::stop_token const &stop_token) noexcept : _stop_token(stop_token), _stop_callback_ptr()
    {
    }

    [[nodiscard]] bool await_ready() const noexcept
    {
        return _stop_token.stop_requested();
    }

    void await_suspend(std::coroutine_handle<> handle) noexcept;

    void await_resume() noexcept {
        _stop_callback_ptr = nullptr;
    }

private:
    struct callback_wrapper {
        void operator()() noexcept;

        loop *await_loop;
        std::coroutine_handle<> handle;
    };

    std::stop_token _stop_token;
    std::unique_ptr<std::stop_callback<callback_wrapper>> _stop_callback_ptr;
};

template<>
struct awaitable_cast<std::stop_token> {
    awaitable_stop_token operator()(std::stop_token const& rhs) noexcept
    {
        return awaitable_stop_token{rhs};
    }
};

} // namespace hi::inline v1
