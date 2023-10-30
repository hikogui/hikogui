// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "group_ptr.hpp"
#include "../coroutine/coroutine.hpp"
#include "../macros.hpp"
#include <memory>
#include <vector>
#include <string>

hi_export_module(hikogui.observer : observed);

hi_export namespace hi::inline v1 {

struct observable_msg {
    /** The type of the path used for notifying observers.
     */
    using path_type = std::vector<std::string>;

    void const * const ptr;
    path_type const& path;
};

/** An abstract observed object.
 *
 * This type is referenced by `observer`s
 */
class observed : public enable_group_ptr<observed, void(observable_msg)> {
public:
    virtual ~observed() = default;
    observed(observed const&) = delete;
    observed(observed&&) = delete;
    observed& operator=(observed const&) = delete;
    observed& operator=(observed&&) = delete;
    constexpr observed() noexcept = default;

    /** Get a pointer to the current value.
     *
     * @note `read()` does not `read_lock()` the observed and should be done before `read()`.
     * @return A const pointer to the value. The `observer` should cast this to a pointer to the value-type.
     */
    [[nodiscard]] virtual void const *read() const noexcept = 0;

    /** Allocate and make a copy of the value.
     *
     * @note `copy()` does not `write_lock()` the observed and should be done before `read()`.
     * @param ptr A pointer to the value that was `read()`.
     * @return A pointer to a newly allocated copy of the value.
     */
    [[nodiscard]] virtual void *copy(void const *ptr) const noexcept = 0;

    /** Commit the modified copy.
     *
     * @note `commit()` does not `write_unlock()`.
     * @param ptr A pointer to the modified new value returned by `copy()`.
     */
    virtual void commit(void *ptr) noexcept = 0;

    /** Abort the modified copy.
     *
     * @note `abort()` does not `write_unlock()`.
     * @param ptr A pointer to the modified new value returned by `copy()`.
     */
    virtual void abort(void *ptr) const noexcept = 0;

    /** Lock for reading.
     */
    virtual void read_lock() const noexcept = 0;

    /** Unlock for reading.
     */
    virtual void read_unlock() const noexcept = 0;

    /** Lock for writing.
     */
    virtual void write_lock() const noexcept = 0;

    /** Unlock for writing.
     */
    virtual void write_unlock() const noexcept = 0;
};

} // namespace hi::inline v1
