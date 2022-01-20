// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../required.hpp"
#include "../assert.hpp"
#include "../math.hpp"
#include <tuple>
#include <cstdlib>
#include <algorithm>

namespace tt::inline v1{

    class text_cursor {
    public:
        constexpr text_cursor() noexcept : _value(0) {}
        constexpr text_cursor(text_cursor const &) noexcept = default;
        constexpr text_cursor(text_cursor &&) noexcept = default;
        constexpr text_cursor &operator=(text_cursor const &) noexcept = default;
        constexpr text_cursor &operator=(text_cursor &&) noexcept = default;

        constexpr text_cursor(size_t index, bool after) noexcept : _value(index << 1 | static_cast<size_t>(after)) {}

        [[nodiscard]] constexpr text_cursor neighbour() const noexcept
        {
            text_cursor r;
            r._value = _value + (after() ? 1 : -1);
            return r;
        }

        [[nodiscard]] constexpr bool start_of_text() const noexcept
        {
            return _value == 0;
        }

        [[nodiscard]] constexpr size_t index() const noexcept
        {
            return _value >> 1;
        }

        [[nodiscard]] constexpr bool after() const noexcept
        {
            return static_cast<bool>(_value & 1);
        }

        [[nodiscard]] constexpr bool before() const noexcept
        {
            return not after();
        }
        
        [[nodiscard]] constexpr friend bool operator==(text_cursor const &, text_cursor const &) = default;
        [[nodiscard]] constexpr friend auto operator<=>(text_cursor const &, text_cursor const &) = default;

    private:
        size_t _value;
    };

}
