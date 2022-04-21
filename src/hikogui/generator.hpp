// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <concepts>
#include <coroutine>
#include <optional>
#include <memory>
#include <memory_resource>
#include "arguments.hpp"

namespace hi::inline v1 {

/** A return value for a generator-function.
 * A generator-function is a coroutine which co_yields zero or more values.
 *
 * The generator object returned from the generator-function is used to retrieve
 * the yielded values through an forward-iterator returned by the
 * `begin()` and `end()` member functions.
 *
 * The incrementing the iterator will resume the generator-function until
 * the generator-function co_yields another value.
 */
template<typename T, typename Allocator = std::allocator<std::byte>>
class generator {
public:
    static_assert(
        std::is_same_v<typename std::allocator_traits<Allocator>::value_type, std::byte>,
        "Allocator's value_type must be std::byte");
    using allocator_type = Allocator;
    using value_type = T;

    class promise_type {
    public:
        struct allocator_info {
            allocator_type *allocator;
            std::size_t size;
        };

        void operator delete(void *ptr)
        {
            if (ptr) {
                auto *ptr_ = reinterpret_cast<std::byte *>(ptr) - sizeof(allocator_info);
                auto *info_ptr = std::launder(reinterpret_cast<allocator_info *>(ptr_));
                auto info = *info_ptr;
                std::destroy_at(info_ptr);

                if constexpr (std::allocator_traits<allocator_type>::is_always_equal::value) {
                    auto allocator = allocator_type{};
                    std::allocator_traits<allocator_type>::deallocate(allocator, ptr_, info.size);
                } else {
                    std::allocator_traits<allocator_type>::deallocate(*(info.allocator), ptr_, info.size);
                }
            }
        }

        [[nodiscard]] static void *simple_new(std::size_t size)
        {
            auto size_plus_info = size + sizeof(allocator_info);

            // Prefix the allocated memory with the allocated size.
            auto allocator = allocator_type{};
            auto *ptr = std::allocator_traits<allocator_type>::allocate(allocator, size_plus_info);
            [[maybe_unused]] auto *info = new (ptr) allocator_info(nullptr, size_plus_info);
            return ptr + sizeof(allocator_info);
        }

        [[nodiscard]] static void *allocator_new(std::size_t size, allocator_type &allocator)
        {
            auto size_plus_info = size + sizeof(allocator_info);

            // Allocator is in the trailing argument, because coroutines can be methods
            // and the first argument would be `this`.
            // Prefix the allocated memory with a pointer to the allocator and the allocated size.
            auto *ptr = std::allocator_traits<allocator_type>::allocate(allocator, size_plus_info);
            [[maybe_unused]] auto *info = new (ptr) allocator_info(&allocator, size_plus_info);
            return ptr + sizeof(allocator_info);
        }

        template<typename... Args>
        void *operator new(std::size_t size, Args &&...args)
        {
            if constexpr (std::allocator_traits<allocator_type>::is_always_equal::value) {
                // This is just a coroutine with arguments.
                return promise_type::simple_new(size);
            } else {
                return promise_type::allocator_new(size, get_last_argument(args...));
            }
        }

        generator get_return_object()
        {
            return generator{handle_type::from_promise(*this)};
        }

        value_type const &value()
        {
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

    class value_proxy {
    public:
        value_proxy(value_type const &value) noexcept : _value(value) {}

        value_type const &operator*() const noexcept
        {
            return _value;
        }

    private:
        value_type _value;
    };

    /** A forward iterator which iterates through values co_yieled by the generator-function.
     */
    class iterator {
    public:
        using difference_type = ptrdiff_t;
        using value_type = std::remove_cv_t<value_type>;
        using pointer = value_type *;
        using reference = value_type &;
        using iterator_category = std::input_iterator_tag;

        explicit iterator(handle_type coroutine) : _coroutine{coroutine} {}

        /** Resume the generator-function.
         */
        iterator &operator++()
        {
            _coroutine.resume();
            return *this;
        }

        value_proxy operator++(int)
        {
            auto tmp = value_proxy(**this);
            _coroutine.resume();
            return tmp;
        }

        /** Retrieve the value co_yielded by the generator-function.
         */
        value_type const &operator*() const
        {
            return _coroutine.promise().value();
        }

        value_type const *operator->() const
        {
            return std::addressof(_coroutine.promise().value());
        }

        /** Check if the generator-function has finished.
         */
        [[nodiscard]] bool operator==(std::default_sentinel_t) const
        {
            return (not _coroutine) or _coroutine.done();
        }

    private:
        handle_type _coroutine;
    };

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
        hi_axiom(&other != this);
        other._coroutine = {};
    }

    generator &operator=(generator &&other) noexcept
    {
        hi_return_on_self_assignment(other);
        if (_coroutine) {
            _coroutine.destroy();
        }
        _coroutine = other._coroutine;
        other._coroutine = {};
        return *this;
    }

    /** Start the generator-function and return an iterator.
     */
    iterator begin()
    {
        if (_coroutine) {
            _coroutine.resume();
        }
        return iterator{_coroutine};
    }

    /** Return a sentinal for the iterator.
     */
    std::default_sentinel_t end()
    {
        return {};
    }

private:
    handle_type _coroutine;
};

namespace pmr {

template<typename T>
using generator = hi::generator<T, std::pmr::polymorphic_allocator<>>;

}

} // namespace hi::inline v1
