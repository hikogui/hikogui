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

hi_export namespace hi { inline namespace v1 {

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
class observed_base : public enable_group_ptr<observed, void(observable_msg)> {
public:
    virtual ~observed_base() = default;
    observed_base(observed_base const&) = delete;
    observed_base(observed_base&&) = delete;
    observed_base& operator=(observed_base const&) = delete;
    observed_base& operator=(observed_base&&) = delete;
    constexpr observed_base() noexcept = default;

    /** Get a pointer to the current value.
     *
     * @return A const pointer to the value. The `observer` should cast this to a pointer to the value-type.
     */
    [[nodiscard]] virtual void const *get() const noexcept = 0;

    /** Get a pointer to the current value.
     *
     * @return A pointer to the value. The `observer` should cast this to a pointer to the value-type.
     */
    [[nodiscard]] virtual void *get() noexcept = 0;
};

template<std::equality_comparable T>
class observed final : public observed_base {
public:
    using value_type = T;
    using path_type = observable_msg::path_type;

    ~observed() = default;

    /** Construct the shared state and initialize the value.
     *
     * @param args The arguments passed to the constructor of the value.
     */
    template<typename... Args>
    constexpr observed(Args&&...args) noexcept : observed_base(), _value(std::forward<Args>(args)...)
    {
    }

    [[nodiscard]] void const *get() const noexcept override
    {
        return static_cast<void const *>(std::addressof(_value));
    }

    [[nodiscard]] void *get() noexcept override
    {
        return static_cast<void const *>(std::addressof(_value));
    }

private:
    value_type _value;
};

}} // namespace hi::inline v1
