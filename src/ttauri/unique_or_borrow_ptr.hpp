// Copyright Take Vos 2021
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <memory>

namespace tt {

/** Class that hold either a unique or a borrowed ptr.
 */
template<typename T>
class unique_or_borrow_ptr {
public:
    using value_type = T;
    using pointer = value_type *;

    ~unique_or_borrow_ptr() = default;
    unique_or_borrow_ptr(unique_or_borrow_ptr const &) = delete;
    unique_or_borrow_ptr &operator=(unique_or_borrow_ptr const &) = delete;

    unique_or_borrow_ptr(unique_or_borrow_ptr &&other) noexcept :
        _borrow_ptr(std::exchange(other._borrow_ptr, nullptr)), _unique_ptr(std::move(other._unique_ptr))
    {
    }

    unique_or_borrow_ptr &operator=(unique_or_borrow_ptr &&other) noexcept
    {
        _borrow_ptr = std::exchange(other._borrow_ptr, nullptr);
        _unique_ptr = std::move(other._unique_ptr);
        return *this;
    }

    constexpr unique_or_borrow_ptr() noexcept : _borrow_ptr(nullptr), _unique_ptr() {}

    constexpr unique_or_borrow_ptr(std::nullptr_t) noexcept : _borrow_ptr(nullptr), _unique_ptr() {}

    unique_or_borrow_ptr(value_type *other) noexcept : _borrow_ptr(other), _unique_ptr() {}

    unique_or_borrow_ptr(value_type &other) noexcept : _borrow_ptr(&other), _unique_ptr() {}

    unique_or_borrow_ptr(std::unique_ptr<value_type> &&other) noexcept :
        _borrow_ptr(nullptr), _unique_ptr(std::move(other))
    {
    }

    unique_or_borrow_ptr &operator=(std::nullptr_t) noexcept
    {
        _borrow_ptr = nullptr;
        _unique_ptr = nullptr;
        return *this;
    }

    unique_or_borrow_ptr &operator=(pointer other) noexcept
    {
        _borrow_ptr = other;
        _unique_ptr = nullptr;
        return *this;
    }

    unique_or_borrow_ptr &operator=(std::unique_ptr<value_type> &&other) noexcept
    {
        _borrow_ptr = nullptr;
        _unique_ptr = std::move(other);
        return *this;
    }

    [[nodiscard]] pointer get() const noexcept
    {
        return _borrow_ptr ? _borrow_ptr : _unique_ptr.get();
    }

    explicit operator bool () const noexcept
    {
        return _borrow_ptr or static_cast<bool>(_unique_ptr);
    }

    typename std::add_lvalue_reference<T>::type operator*() const noexcept
    {
        return *get();
    }

    pointer operator->() const noexcept
    {
        return get();
    }

private:
    value_type *_borrow_ptr = nullptr;
    std::unique_ptr<T> _unique_ptr;
};

} // namespace tt
