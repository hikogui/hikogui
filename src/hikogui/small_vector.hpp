// Copyright Take Vos 2019, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "utility/module.hpp"
#include <array>

namespace hi::inline v1 {

template<typename T, std::size_t N>
class small_vector {
    using value_type = T;
    using array_type = std::array<T, N>;
    using iterator = typename array_type::iterator;
    using const_iterator = typename array_type::iterator;

    array_type items;
    iterator _end;

public:
    constexpr small_vector() noexcept
    {
        _end = items.begin();
    }

    constexpr auto begin() noexcept
    {
        return items.begin();
    }

    constexpr auto end() noexcept
    {
        return _end;
    }

    constexpr std::size_t size() const noexcept
    {
        return static_cast<std::size_t>(_end - items.begin());
    }

    constexpr void clear() noexcept
    {
        _end = items.begin();
    }

    constexpr bool push_back(value_type &&value) noexcept
    {
        if (_end == items.end()) {
            return false;
        }
        *(_end++) = std::move(value);
        return true;
    }

    constexpr bool push_back(value_type const &value) noexcept
    {
        if (_end == items.end()) {
            return false;
        }
        *(_end++) = value;
        return true;
    }
};

} // namespace hi::inline v1