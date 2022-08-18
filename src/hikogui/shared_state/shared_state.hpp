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

template<typename T>
class shared_state final : public shared_state_base {
public:
    using value_type = T;

    ~shared_state() = default;
    constexpr shared_state() noexcept = default;

    template<typename... Args>
    void emplace(Args&&...args)
    {
        _rcu.emplace(std::forward<Args>(args)...);
    }

    shared_state_cursor<value_type> cursor() && = delete;
    [[nodiscard]] auto operator[](auto const& index) && = delete;
    template<basic_fixed_string Name>
    [[nodiscard]] auto _() && = delete;

    [[nodiscard]] shared_state_cursor<value_type> cursor() const& noexcept
    {
        return {const_cast<shared_state *>(this), std::string{}, [](void *base) -> void * {
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

    [[nodiscard]] void const *read() noexcept override
    {
        return _rcu.get();
    }

    [[nodiscard]] void *copy() noexcept override
    {
        return _rcu.copy();
    }

    void commit(void *ptr, std::string const& path) noexcept override
    {
        _rcu.commit(static_cast<value_type *>(ptr));
        notify(path);
    }

    void abort(void *ptr) noexcept override
    {
        _rcu.abort(static_cast<value_type *>(ptr));
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
