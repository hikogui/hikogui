// Copyright Take Vos 2021
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <memory>

namespace tt {

/** Class that hold either a weak_ptr or a shared_ptr.
 * This class is to hold a weak_ptr, and potentially also
 * an owning shared_ptr.
 */
template<typename T>
class weak_or_shared_ptr {
public:
    using value_type = T;
    using pointer = value_type *;

    ~weak_or_shared_ptr() = default;
    weak_or_shared_ptr(weak_or_shared_ptr const &) = default;
    weak_or_shared_ptr(weak_or_shared_ptr &&) = default;
    weak_or_shared_ptr &operator=(weak_or_shared_ptr const &) = default;
    weak_or_shared_ptr &operator=(weak_or_shared_ptr &&) = default;
    constexpr weak_or_shared_ptr() noexcept = default;

    constexpr weak_or_shared_ptr(std::nullptr_t) noexcept : weak_or_shared_ptr() {}

    weak_or_shared_ptr &operator=(std::nullptr_t) noexcept
    {
        _shared_ptr = nullptr;
        _weak_ptr = nullptr;
        return *this;
    }

    explicit weak_or_shared_ptr(std::shared_ptr<value_type> const &other) noexcept :
        _shared_ptr(other), _weak_ptr(other) {}

    explicit weak_or_shared_ptr(std::shared_ptr<value_type> &&other) noexcept :
        _shared_ptr(std::move(other)), _weak_ptr(_shared_ptr) {}

    weak_or_shared_ptr(std::weak_ptr<value_type> const &other) noexcept :
        _shared_ptr(), _weak_ptr(other) {}

    weak_or_shared_ptr(std::weak_ptr<value_type> &&other) noexcept :
        _shared_ptr(), _weak_ptr(std::move(other)) {}

    weak_or_shared_ptr &operator=(std::weak_ptr<value_type> const &other) noexcept
    {
        _shared_ptr = nullptr;
        _weak_ptr = other;
        return *this;
    }

    weak_or_shared_ptr &operator=(std::weak_ptr<value_type> &&other) noexcept
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

    [[nodiscard]] long use_count() const noexcept
    {
        return _weak_ptr.use_count();
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
};

} // namespace tt
