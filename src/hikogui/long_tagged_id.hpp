// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "stdint.hpp"
#include "utility/module.hpp"
#include "sip_hash.hpp"
#include <bit>

namespace hi::inline v1 {

template<fixed_string Tag>
class long_tagged_id {
public:
    constexpr static auto tag = Tag;

    constexpr long_tagged_id() noexcept = default;
    constexpr long_tagged_id(long_tagged_id const&) noexcept = default;
    constexpr long_tagged_id(long_tagged_id&&) noexcept = default;
    constexpr long_tagged_id& operator=(long_tagged_id const&) noexcept = default;
    constexpr long_tagged_id& operator=(long_tagged_id&&) noexcept = default;

    constexpr long_tagged_id(nullptr_t) noexcept : _value() {}
    constexpr long_tagged_id& operator=(nullptr_t) noexcept
    {
        _value = {};
        return *this;
    }

    template<typename... Args>
    constexpr explicit long_tagged_id(Args const&...args) noexcept :
        _value()
    {
        auto tmp = (... ^ sip_hash24x2<Args>{}(args));
        _value = std::bit_cast<uint128_t>(tmp);
        if (_value == 0) {
            _value = 1;
        }
    }

    template<typename T>
    constexpr long_tagged_id& operator=(T const& other) noexcept
    {
        _value = std::bit_cast<uint128_t>(sip_hash24x2<std::decay_t<T>>{}(other));
        if (_value == 0) {
            _value = 1;
        }
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return _value == 0;
    }

    constexpr operator bool() const noexcept
    {
        return not empty();
    }

    [[nodiscard]] constexpr friend bool operator==(long_tagged_id const& lhs, long_tagged_id const& rhs) noexcept = default;

private:
    uint128_t _value = 0;
};

} // namespace hi::inline v1
