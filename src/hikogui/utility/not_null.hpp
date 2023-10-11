// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "assert.hpp"
#include "../macros.hpp"
#include <type_traits>
#include <memory>

hi_export_module(utility.not_null);

namespace hi { inline namespace v1 {

template<typename T>
class not_null;

template<typename T>
class not_null<T *> {
public:
    using pointer = T *;
    using reference = T&;

    constexpr not_null(not_null const&) noexcept = default;
    constexpr not_null(not_null&&) noexcept = default;
    constexpr not_null& operator=(not_null const&) noexcept = default;
    constexpr not_null& operator=(not_null&&) noexcept = default;
    not_null() = delete;
    not_null(nullptr_t) = delete;
    not_null& operator=(nullptr_t) = delete;

    constexpr not_null(pointer other) noexcept : _p(other)
    {
        hi_assert_not_null(other);
    }

    constexpr not_null& operator=(pointer other) noexcept
    {
        hi_assert_not_null(other);
        _p = other;
        return *this;
    }

    constexpr operator pointer() const noexcept
    {
        return _p;
    }

    constexpr pointer get() const noexcept
    {
        return _p;
    }

    constexpr pointer operator->() const noexcept
    {
        return _p;
    }

    constexpr reference operator*() const noexcept
    {
        return *_p;
    }

    bool operator==(nullptr_t) const = delete;

    [[nodiscard]] constexpr bool operator==(std::remove_const_t<T> const *rhs) const noexcept
    {
        return _p == rhs;
    }

private:
    pointer _p;
};

template<typename T>
class not_null<std::unique_ptr<T>> {
public:
    using pointer = T *;
    using reference = T&;

    not_null(not_null const&) noexcept = delete;
    not_null(not_null&&) noexcept = default;
    not_null& operator=(not_null const&) noexcept = delete;
    not_null& operator=(not_null&&) noexcept = default;
    not_null() = delete;
    not_null(nullptr_t) = delete;
    not_null& operator=(nullptr_t) = delete;

    not_null(std::unique_ptr<T> other) noexcept : _p(std::move(other))
    {
        hi_assert_not_null(_p);
    }

    not_null& operator=(std::unique_ptr<T> other) noexcept
    {
        hi_assert_not_null(other);
        _p = std::move(other);
        return *this;
    }

    operator std::unique_ptr<T> const &() const noexcept
    {
        return _p;
    }

    operator std::unique_ptr<T> &() noexcept
    {
        return _p;
    }

    pointer get() const noexcept
    {
        return _p.get();
    }

    pointer operator->() const noexcept
    {
        return _p.get();
    }

    reference operator*() const noexcept
    {
        return *_p;
    }

    [[nodiscard]] bool operator==(nullptr_t) const = delete;
    
    [[nodiscard]] bool operator==(std::remove_const_t<T> const *rhs) const noexcept
    {
        return _p == rhs;
    }

    [[nodiscard]] bool operator==(std::unique_ptr<std::remove_const_t<T>> rhs) const = delete;
    [[nodiscard]] bool operator==(std::unique_ptr<std::remove_const_t<T> const> rhs) const = delete;

private:
    std::unique_ptr<T> _p;

    friend not_null<std::shared_ptr<std::remove_const_t<T>>>;
    friend not_null<std::shared_ptr<std::remove_const_t<T> const>>;
};

template<typename T>
class not_null<std::shared_ptr<T>> {
public:
    using pointer = T *;
    using reference = T&;

    constexpr not_null(not_null const&) noexcept = default;
    constexpr not_null(not_null&&) noexcept = default;
    constexpr not_null& operator=(not_null const&) noexcept = default;
    constexpr not_null& operator=(not_null&&) noexcept = default;
    not_null() = delete;
    not_null(nullptr_t) = delete;
    not_null& operator=(nullptr_t) = delete;

    constexpr not_null(not_null<std::unique_ptr<std::remove_const_t<T>>> other) noexcept : _p(std::move(other._p)) {}

    constexpr not_null& operator=(not_null<std::unique_ptr<std::remove_const_t<T>>> other) noexcept
    {
        _p = std::move(other._p);
        return *this;
    }

    constexpr not_null(not_null<std::unique_ptr<std::remove_const_t<T> const>> other) noexcept
        requires(std::is_const_v<T>)
        : _p(std::move(other._p))
    {
    }

    constexpr not_null& operator=(not_null<std::unique_ptr<std::remove_const_t<T> const>> other) noexcept
        requires(std::is_const_v<T>)
    {
        _p = std::move(other._p);
        return *this;
    }

    constexpr not_null(std::shared_ptr<T> other) noexcept : _p(std::move(other))
    {
        hi_assert_not_null(_p);
    }

    constexpr not_null& operator=(std::shared_ptr<T> other) noexcept
    {
        hi_assert_not_null(other);
        _p = std::move(other);
        return *this;
    }

    operator std::shared_ptr<T> const &() const noexcept
    {
        return _p;
    }

    operator std::shared_ptr<T> &() noexcept
    {
        return _p;
    }

    constexpr pointer get() const noexcept
    {
        return _p.get();
    }

    constexpr pointer operator->() const noexcept
    {
        return _p.get();
    }

    constexpr reference operator*() const noexcept
    {
        return *_p;
    }

    [[nodiscard]] bool operator==(nullptr_t) const = delete;
    
    [[nodiscard]] bool operator==(std::remove_const_t<T> const *rhs) const noexcept
    {
        return _p == rhs;
    }

    [[nodiscard]] bool operator==(std::shared_ptr<std::remove_const_t<T>> const &rhs) const noexcept
    {
        return _p == rhs;
    }

    [[nodiscard]] bool operator==(std::shared_ptr<std::remove_const_t<T> const> const &rhs) const noexcept
    {
        return _p == rhs;
    }

private:
    std::shared_ptr<T> _p;
};

}} // namespace hi::v1
