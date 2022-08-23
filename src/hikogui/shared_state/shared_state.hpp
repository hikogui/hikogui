// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "shared_state_base.hpp"
#include "shared_state_cursor.hpp"
#include "../rcu.hpp"
#include "../notifier.hpp"
#include <string_view>

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

    /** Construct the shared state and initialize the value.
     *
     * @param args The arguments passed to the constructor of the value.
     */
    template<typename... Args>
    constexpr shared_state(Args &&... args) noexcept : _rcu()
    {
        _rcu.emplace(std::forward<Args>(args)...);
    }

    // clang-format off
    shared_state_cursor<value_type> cursor() && = delete;
    [[nodiscard]] auto operator[](auto const& index) && = delete;
    template<basic_fixed_string Name> [[nodiscard]] auto _() && = delete;
    // clang-format on

    /** Get a cursor to the value.
     *
     * This function returns a cursor to the value object.
     * The cursor is used to start read or write transactions or create other cursors.
     *
     * @return The new cursor pointing to the value object.
     */
    [[nodiscard]] shared_state_cursor<value_type> cursor() const& noexcept
    {
        auto new_path = std::vector<std::string>{{"/"}};
        return {const_cast<shared_state *>(this), std::move(new_path), [](void *base) -> void * {
                    return base;
                }};
    }

    /** Get a cursor to a sub-object of value accessed by the index operator.
     *
     * @param index The index used with the index operator of the value.
     * @return The new cursor pointing to a sub-object of the value.
     */
    [[nodiscard]] auto operator[](auto const& index) const& noexcept
        requires requires { cursor()[index]; }
    {
        return cursor()[index];
    }

    /** Get a cursor to a member variable of the value.
     *
     * @note This requires the specialization of `hi::selector<value_type>`.
     * @tparam Name the name of the member variable of the value.
     * @return The new cursor pointing to the member variable of the value
     */
    template<basic_fixed_string Name>
    [[nodiscard]] auto get() const& noexcept
    {
        return cursor().get<Name>();
    }

private:
    using path_type = std::vector<std::string>;

    rcu<value_type> _rcu;
    unfair_mutex _write_mutex;

    [[nodiscard]] void const *read() noexcept override
    {
        return _rcu.get();
    }

    [[nodiscard]] void *copy() noexcept override
    {
        _write_mutex.lock();
        return _rcu.copy();
    }

    void commit(void *ptr, path_type const& path) noexcept override
    {
        _rcu.commit(static_cast<value_type *>(ptr));
        _write_mutex.unlock();
        notify(path);
    }

    void abort(void *ptr) noexcept override
    {
        _rcu.abort(static_cast<value_type *>(ptr));
        _write_mutex.unlock();
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
