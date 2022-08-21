// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "shared_state_base.hpp"
#include "shared_state_cursor.hpp"
#include "../rcu.hpp"
#include "../notifier.hpp"
#include <string_view>
#include <map>

namespace hi::inline v1 {

/** Shared state of an application.
 *
 * The shared state of an application that can be manipulated by the GUI,
 * preference and other systems.
 *
 * A `shared_cursor` selects a member or indexed element from the shared state,
 * or from another cursor. You can `.read()` or `.copy()` the value pointed to
 * by the cursor to read and manipulate the shared-data.
 *
 * Both `.read()` and `.copy()` take the full shared-state as a whole not allowing
 * other threads to have write access to this reference or copy. A copy will be
 * automatically committed, or may be aborted as well.
 * 
 * lifetime:
 * - The lifetime of `shared_cursor` must be within the lifetime of `shared_state`.
 * - The lifetime of `shared_cursor::proxy` must be within the lifetime of `shared_cursor`.
 * - The lifetime of `shared_cursor::const_proxy` must be within the lifetime of `shared_cursor`.
 * - Although `shared_cursor` are created from another `shared_cursor` they internally do not
 *   refer to each other so their lifetime are not connected.
 *
 * @tparam T type used as the shared state.
 */
template<typename T>
class shared_state final : public shared_state_base {
public:
    using value_type = T;

    ~shared_state() = default;

    template<typename... Args>
    constexpr shared_state(Args &&... args) noexcept : _rcu()
    {
        _rcu.emplace(std::forward<Args>(args)...);
    }

    shared_state_cursor<value_type> cursor() && = delete;

    [[nodiscard]] auto operator[](auto const& index) && = delete;

    template<basic_fixed_string Name>
    [[nodiscard]] auto _() && = delete;

    [[nodiscard]] shared_state_cursor<value_type> cursor() const& noexcept
    {
        return {const_cast<shared_state *>(this), "/", [](void *base) -> void * {
                    return base;
                }};
    }

    [[nodiscard]] auto operator[](auto const& index) const& noexcept
    {
        return cursor()[index];
    }

    template<basic_fixed_string Name>
    [[nodiscard]] auto _() const& noexcept
    {
        return cursor()._<Name>();
    }

private:
    rcu<value_type> _rcu;
    std::atomic<bool> _writing;

    [[nodiscard]] void const *read() noexcept override
    {
        return _rcu.get();
    }

    [[nodiscard]] void *copy() noexcept override
    {
#ifndef NDEBUG
        hi_assert(_writing.exchange(true, std::memory_order::acquire) == false);
#endif
        return _rcu.copy();
    }

    void commit(void *ptr, std::string const& path) noexcept override
    {
        _rcu.commit(static_cast<value_type *>(ptr));
        notify(path);
#ifndef NDEBUG
        hi_assert(_writing.exchange(false, std::memory_order::acquire) == true);
#endif
    }

    void abort(void *ptr) noexcept override
    {
        _rcu.abort(static_cast<value_type *>(ptr));
#ifndef NDEBUG
        hi_assert(_writing.exchange(false, std::memory_order::acquire) == true);
#endif
    }

    void lock() noexcept override
    {
        _rcu.lock();
    }

    void unlock() noexcept override
    {
        _rcu.unlock();
    }
};

} // namespace hi::inline v1
