// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../notifier.hpp"
#include <string_view>
#include <map>

namespace hi::inline v1 {

class shared_state_base {
public:
    using notifier_type = notifier<>;
    using token_type = notifier_type::token_type;
    using function_type = notifier_type::function_type;

    constexpr ~shared_state_base() = default;
    shared_state_base(shared_state_base const&) = delete;
    shared_state_base(shared_state_base&&) = delete;
    shared_state_base& operator=(shared_state_base const&) = delete;
    shared_state_base& operator=(shared_state_base&&) = delete;
    constexpr shared_state_base() noexcept = default;

protected:
    std::map<std::string, notifier_type> _notifiers;

    [[nodiscard]] virtual void const *read() noexcept = 0;
    [[nodiscard]] virtual void *copy() noexcept = 0;
    virtual void commit(void *ptr, std::string const& path) noexcept = 0;
    virtual void abort(void *ptr) noexcept = 0;
    virtual void lock() noexcept = 0;
    virtual void unlock() noexcept = 0;

    [[nodiscard]] token_type subscribe(std::string const& path, callback_flags flags, function_type function) noexcept
    {
        auto [it, inserted] = _notifiers.emplace(path, notifier_type{});
        return it->second.subscribe(flags, std::move(function));
    }

    void notify(std::string const& path) noexcept
    {
        for (auto it = _notifiers.lower_bound(path); it != _notifiers.end() and it->first.starts_with(path); ++it) {
            it->second();
        }
    }

    template<typename O>
    friend class shared_state_cursor;
};

}
