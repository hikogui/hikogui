// Copyright Take Vos 2021
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <memory>

namespace tt::inline v1 {

/** Class that hold either a weak_ptr or a unique_ptr
 * This class is to hold a weak_ptr, and potentially also
 * an owning unique_ptr. Technically this class will use
 * a shared_ptr for owning.
 *
 * The following unique_ptr, shared_ptr, weak_ptr or weak_or_unique_ptr conversions are allowed:
 *  - move weak_or_unique_ptr: (optional) ownership is moved
 *  - copy weak_or_unique_ptr: weak_ptr is copied
 *  - move unique_ptr: ownership is moved
 *  - copy shared_ptr: weak_ptr is copied
 *  - move weak_ptr: weak_ptr is moved
 *  - copy weak_ptr: weak_ptr is copied
 */
template<typename T>
class weak_or_unique_ptr {
public:
    using value_type = T;
    using pointer = value_type *;

    ~weak_or_unique_ptr() = default;
    constexpr weak_or_unique_ptr() noexcept = default;
    constexpr weak_or_unique_ptr(std::nullptr_t) noexcept : weak_or_unique_ptr() {}

    weak_or_unique_ptr &operator=(std::nullptr_t) noexcept
    {
        _shared_ptr = {};
        _weak_ptr = {};
        return *this;
    }

    template<typename Y>
    requires(std::is_convertible_v<std::remove_cvref_t<Y> *, value_type *>) weak_or_unique_ptr(weak_or_unique_ptr<Y> const &other)
    noexcept : _shared_ptr(), _weak_ptr(other._weak_ptr) {}

    template<typename Y>
    requires(std::is_convertible_v<std::remove_cvref_t<Y> *, value_type *>) weak_or_unique_ptr &
    operator=(weak_or_unique_ptr<Y> const &other) noexcept
    {
        _shared_ptr = {};
        _weak_ptr = other._weak_ptr;
        return *this;
    }

    template<typename Y>
    requires(std::is_convertible_v<std::remove_cvref_t<Y> *, value_type *>) weak_or_unique_ptr(weak_or_unique_ptr<Y> &&other)
    noexcept : _shared_ptr(other._shared_ptr), _weak_ptr(other._weak_ptr) {}

    template<typename Y>
    requires(std::is_convertible_v<std::remove_cvref_t<Y> *, value_type *>) weak_or_unique_ptr &
    operator=(weak_or_unique_ptr<Y> &&other) noexcept
    {
        _shared_ptr = other._shared_ptr;
        _weak_ptr = other._weak_ptr;
        return *this;
    }

    template<typename Y>
    requires(std::is_convertible_v<std::remove_cvref_t<Y> *, value_type *>) weak_or_unique_ptr(std::unique_ptr<Y> &&other)
    noexcept : _shared_ptr(std::move(other)), _weak_ptr(_shared_ptr) {}

    template<typename Y>
    requires(std::is_convertible_v<std::remove_cvref_t<Y> *, value_type *>) weak_or_unique_ptr &
    operator=(std::unique_ptr<Y> &&other) noexcept
    {
        _shared_ptr = std::move(other);
        _weak_ptr = _shared_ptr;
        return *this;
    }

    template<typename Y>
    requires(std::is_convertible_v<std::remove_cvref_t<Y> *, value_type *>) weak_or_unique_ptr(std::shared_ptr<Y> const &other)
    noexcept : _shared_ptr(), _weak_ptr(other) {}

    template<typename Y>
    requires(std::is_convertible_v<std::remove_cvref_t<Y> *, value_type *>) weak_or_unique_ptr &
    operator=(std::shared_ptr<Y> const &other) noexcept
    {
        _shared_ptr = nullptr;
        _weak_ptr = other;
        return *this;
    }

    template<typename Y>
    requires(std::is_convertible_v<std::remove_cvref_t<Y> *, value_type *>) weak_or_unique_ptr(std::weak_ptr<Y> const &other)
    noexcept : _shared_ptr(), _weak_ptr(other) {}

    template<typename Y>
    requires(std::is_convertible_v<std::remove_cvref_t<Y> *, value_type *>) weak_or_unique_ptr(std::weak_ptr<Y> &&other)
    noexcept : _shared_ptr(), _weak_ptr(std::move(other)) {}

    template<typename Y>
    requires(std::is_convertible_v<std::remove_cvref_t<Y> *, value_type *>) weak_or_unique_ptr &
    operator=(std::weak_ptr<Y> const &other) noexcept
    {
        _shared_ptr = nullptr;
        _weak_ptr = other;
        return *this;
    }

    template<typename Y>
    requires(std::is_convertible_v<std::remove_cvref_t<Y> *, value_type *>) weak_or_unique_ptr &
    operator=(std::weak_ptr<Y> &&other) noexcept
    {
        _shared_ptr = nullptr;
        _weak_ptr = std::move(other);
        return *this;
    }

    void reset() noexcept
    {
        _shared_ptr = nullptr;
        _weak_ptr = nullptr;
    }

    [[nodiscard]] bool expired() const noexcept
    {
        return _weak_ptr.expired();
    }

    [[nodiscard]] std::shared_ptr<value_type> lock() const noexcept
    {
        return _weak_ptr.lock();
    }

private:
    std::shared_ptr<T> _shared_ptr;
    std::weak_ptr<T> _weak_ptr;

    template<typename>
    friend class weak_or_unique_ptr;
};

} // namespace tt::inline v1
