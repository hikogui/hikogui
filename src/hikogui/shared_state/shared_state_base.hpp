// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../notifier.hpp"
#include "../tree.hpp"
#include <string>

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
    using path_type = std::vector<std::string>;

    tree<std::string, notifier_type> _notifiers;

    [[nodiscard]] virtual void const *read() noexcept = 0;
    [[nodiscard]] virtual void *copy() noexcept = 0;
    virtual void commit(void *ptr, path_type const& path) noexcept = 0;
    virtual void abort(void *ptr) noexcept = 0;
    virtual void lock() noexcept = 0;
    virtual void unlock() noexcept = 0;

    [[nodiscard]] token_type subscribe(path_type const& path, callback_flags flags, function_type function) noexcept
    {
        auto &notifier = _notifiers[path];
        return notifier.subscribe(flags, std::move(function));
    }

    void notify(path_type const& path) noexcept
    {
        _notifiers.walk_including_path(path, [](notifier_type &notifier) {
            notifier();
        });
    }

    template<typename O>
    friend class shared_state_cursor;
};

}
