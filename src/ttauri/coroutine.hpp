// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include <ranges>
#include <concepts>
#include <coroutine>

namespace tt {

template<typename T>
class generator {
public:
    using value_type = T;

    class promise_type {
    public:
        generator<value_type> get_return_object()
        {
            return generator{handle_type::from_promise(*this)};
        }

        value_type const &value() {
            return *_value;
        }

        static std::suspend_always initial_suspend() noexcept
        {
            return {};
        }

        static std::suspend_always final_suspend() noexcept
        {
            return {};
        }

        std::suspend_always yield_value(value_type const &value) noexcept
        {
            _value = value;
            return {};
        }

        std::suspend_always yield_value(value_type &&value) noexcept
        {
            _value = std::move(value);
            return {};
        }

        void return_void() noexcept {}

        // Disallow co_await in generator coroutines.
        void await_transform() = delete;

        [[noreturn]] static void unhandled_exception()
        {
            throw;
        }

    private:
        std::optional<value_type> _value;
    };

    using handle_type = std::coroutine_handle<promise_type>;

    explicit generator(handle_type coroutine) : _coroutine(coroutine) {}

    generator() = default;
    ~generator()
    {
        if (_coroutine) {
            _coroutine.destroy();
        }
    }

    generator(const generator &) = delete;
    generator &operator=(const generator &) = delete;

    generator(generator &&other) noexcept : _coroutine{other._coroutine}
    {
        other._coroutine = {};
    }

    generator &operator=(generator &&other) noexcept
    {
        if (this != &other) {
            if (_coroutine) {
                _coroutine.destroy();
            }
            _coroutine = other._coroutine;
            other._coroutine = {};
        }
        return *this;
    }

    // Range-based for loop support.
    class iterator {
    public:
        explicit iterator(handle_type coroutine) : _coroutine{coroutine} {}

        iterator &operator++()
        {
            _coroutine.resume();
            return *this;
        }

        value_type const &operator*() const
        {
            return _coroutine.promise().value();
        }

        [[nodiscard]] bool operator==(std::default_sentinel_t) const
        {
            return !_coroutine || _coroutine.done();
        }


    private:
        handle_type _coroutine;
    };

    iterator begin()
    {
        if (_coroutine) {
            _coroutine.resume();
        }
        return iterator{_coroutine};
    }

    std::default_sentinel_t end()
    {
        return {};
    }

private:
    handle_type _coroutine;
};

} // namespace tt
