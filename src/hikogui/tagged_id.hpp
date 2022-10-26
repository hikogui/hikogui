// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "utility.hpp"
#include "cast.hpp"
#include "math.hpp"
#include "fixed_string.hpp"
#include "concepts.hpp"
#include <limits>
#include <typeinfo>
#include <typeindex>
#include <compare>
#include <concepts>

namespace hi::inline v1 {

template<std::unsigned_integral T, fixed_string Tag, ssize_t Max = std::numeric_limits<T>::max() - 1>
class tagged_id {
public:
    static_assert(Max < std::numeric_limits<T>::max(), "Max must be at least one less than the maximum value of T");

    using value_type = T;

    constexpr static value_type max = Max;
    constexpr static value_type invalid = max + 1;
    constexpr static value_type mask = static_cast<value_type>((1ULL << std::bit_width(invalid)) - 1);

    constexpr tagged_id() noexcept : value(invalid) {}
    constexpr tagged_id(tagged_id const &other) noexcept = default;
    constexpr tagged_id(tagged_id &&other) noexcept = default;
    constexpr tagged_id &operator=(tagged_id const &other) noexcept = default;
    constexpr tagged_id &operator=(tagged_id &&other) noexcept = default;

    constexpr explicit tagged_id(numeric_integral auto rhs) noexcept : value(narrow_cast<value_type>(rhs))
    {
        hi_axiom(holds_invariant() and value != invalid);
    }

    constexpr tagged_id &operator=(numeric_integral auto rhs) noexcept
    {
        value = narrow_cast<value_type>(rhs);
        hi_axiom(holds_invariant() and value != invalid);
        return *this;
    }

    constexpr tagged_id &operator=(std::monostate) noexcept
    {
        value = invalid;
        return *this;
    }

    template<numeric_integral O>
    constexpr explicit operator O() const noexcept
    {
        hi_axiom(value != invalid);
        return narrow_cast<O>(value);
    }

    constexpr explicit operator bool() const noexcept
    {
        return value != invalid;
    }

    [[nodiscard]] constexpr value_type const &operator*() const noexcept
    {
        return value;
    }

    [[nodiscard]] constexpr std::size_t hash() const noexcept
    {
        return std::hash<value_type>{}(value);
    }

    [[nodiscard]] constexpr friend bool operator==(tagged_id const &lhs, tagged_id const &rhs) noexcept
    {
        return lhs.value == rhs.value;
    }

    [[nodiscard]] constexpr friend std::partial_ordering operator<=>(tagged_id const &lhs, tagged_id const &rhs) noexcept
    {
        if (lhs.value == invalid or rhs.value == invalid) {
            return std::partial_ordering::unordered;
        } else {
            return lhs.value <=> rhs.value;
        }
    }

    [[nodiscard]] constexpr friend bool operator==(tagged_id const &lhs, std::integral auto const &rhs) noexcept
    {
        return lhs == tagged_id{rhs};
    }

    [[nodiscard]] constexpr friend bool operator==(numeric_integral auto const &lhs, tagged_id const &rhs) noexcept
    {
        return tagged_id{lhs} == rhs;
    }

    [[nodiscard]] constexpr friend std::partial_ordering
    operator<=>(tagged_id const &lhs, numeric_integral auto const &rhs) noexcept
    {
        return lhs <=> tagged_id{rhs};
    }

    [[nodiscard]] constexpr friend std::partial_ordering
    operator<=>(numeric_integral auto const &lhs, tagged_id const &rhs) noexcept
    {
        return tagged_id{lhs} <=> rhs;
    }

    [[nodiscard]] bool holds_invariant() const noexcept
    {
        return value <= max or value == invalid;
    }

    [[nodiscard]] friend std::string to_string(tagged_id const &rhs) noexcept
    {
        return std::format("{}:{}", Tag, rhs.value);
    }

    friend std::ostream &operator<<(std::ostream &lhs, tagged_id const &rhs)
    {
        return lhs << to_string(rhs);
    }

private:
    value_type value;
};

} // namespace hi::inline v1

template<typename T, hi::fixed_string Tag, hi::ssize_t Max>
struct std::hash<hi::tagged_id<T, Tag, Max>> {
    [[nodiscard]] constexpr std::size_t operator()(hi::tagged_id<T, Tag, Max> const &rhs) const noexcept
    {
        return rhs.hash();
    }
};
