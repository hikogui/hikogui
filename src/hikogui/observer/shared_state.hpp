// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "observed_value.hpp"
#include "../utility/utility.hpp"
#include "../concurrency/thread.hpp" // XXX #616
#include "../macros.hpp"
#include <memory>

hi_export_module(hikogui.observer : shared_state);

hi_export namespace hi::inline v1 {

/** Shared state of an application.
 *
 * The shared state of an application that can be manipulated by the GUI,
 * preference and other systems.
 *
 * A `observer` selects a member or indexed element from the shared state,
 * or from another observer. You can `.read()` or `.copy()` the value pointed to
 * by the observer to read and manipulate the shared-data.
 *
 * Both `.read()` and `.copy()` take the full shared-state as a whole not allowing
 * other threads to have write access to this reference or copy. A copy will be
 * automatically committed, or may be aborted as well.
 *
 * lifetime:
 * - The lifetime of `observer` will extend the lifetime of `shared_state`.
 * - The lifetime of `observer::proxy` must be within the lifetime of `observer`.
 * - The lifetime of `observer::const_proxy` must be within the lifetime of `observer`.
 * - Although `observer` are created from another `observer` they internally do not
 *   refer to each other so their lifetime are not connected.
 *
 * @tparam T type used as the shared state.
 */
template<typename T>
class shared_state {
public:
    using value_type = T;

    ~shared_state() = default;
    constexpr shared_state(shared_state const&) noexcept = default;
    constexpr shared_state(shared_state&&) noexcept = default;
    constexpr shared_state& operator=(shared_state const&) noexcept = default;
    constexpr shared_state& operator=(shared_state&&) noexcept = default;

    /** Construct the shared state and initialize the value.
     *
     * @param args The arguments passed to the constructor of the value.
     */
    template<typename... Args>
    constexpr shared_state(Args&&...args) noexcept :
        _pimpl(std::make_shared<observed_value<value_type>>(std::forward<Args>(args)...))
    {
    }

    observer<value_type> observer() const & noexcept
    {
        return ::hi::observer<value_type>(_pimpl);
    }

    // clang-format off
    /** Get a observer to a sub-object of value accessed by the index operator.
     *
     * @param index The index used with the index operator of the value.
     * @return The new observer pointing to a sub-object of the value.
     */
    [[nodiscard]] auto get(auto const& index) const& noexcept
        requires requires { observer().get(index); }
    {
        return observer().get(index);
    }
    // clang-format on

    /** Get a observer to a member variable of the value.
     *
     * @note This requires the specialization of `hi::selector<value_type>`.
     * @tparam Name the name of the member variable of the value.
     * @return The new observer pointing to the member variable of the value
     */
    template<fixed_string Name>
    [[nodiscard]] auto get() const& noexcept
    {
        return observer().template get<Name>();
    }

private:
    std::shared_ptr<observed_value<value_type>> _pimpl;
};

}
