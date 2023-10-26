// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <limits>
#include <typeinfo>
#include <typeindex>
#include <compare>
#include <concepts>
#include <bit>
#include <optional>
#include <string>
#include <ostream>
#include <cstddef>

export module hikogui_utility_tagged_id;
import hikogui_utility_debugger;
import hikogui_utility_fixed_string;

export namespace hi::inline v1 {

template<std::unsigned_integral T, fixed_string Tag, std::size_t Max = std::numeric_limits<T>::max() - 1>
class tagged_id {
public:
    static_assert(Max < std::numeric_limits<T>::max(), "Max must be at least one less than the maximum value of T");

    using value_type = T;

    constexpr static value_type max = Max;
    constexpr static value_type invalid = max + 1;
    constexpr static value_type mask = static_cast<value_type>((1ULL << std::bit_width(invalid)) - 1);

    constexpr tagged_id() noexcept : value(invalid) {}
    constexpr tagged_id(tagged_id const& other) noexcept = default;
    constexpr tagged_id(tagged_id&& other) noexcept = default;
    constexpr tagged_id& operator=(tagged_id const& other) noexcept = default;
    constexpr tagged_id& operator=(tagged_id&& other) noexcept = default;

    constexpr explicit tagged_id(std::integral auto rhs) noexcept : value(narrow_cast<value_type>(rhs))
    {
        hi_axiom(holds_invariant() and value != invalid);
    }

    constexpr tagged_id(std::nullopt_t) noexcept : value(invalid) {}

    constexpr tagged_id(nullptr_t) noexcept : value(invalid) {}

    constexpr tagged_id& operator=(std::integral auto rhs) noexcept
    {
        value = narrow_cast<value_type>(rhs);
        hi_axiom(holds_invariant() and value != invalid);
        return *this;
    }

    constexpr tagged_id& operator=(std::nullopt_t) noexcept
    {
        value = invalid;
        return *this;
    }

    constexpr tagged_id& operator=(nullptr_t) noexcept
    {
        value = invalid;
        return *this;
    }

    template<std::integral O>
    constexpr explicit operator O() const noexcept
    {
        hi_axiom(value != invalid);
        return narrow_cast<O>(value);
    }

    constexpr explicit operator bool() const noexcept
    {
        return value != invalid;
    }

    [[nodiscard]] constexpr value_type const& operator*() const noexcept
    {
        return value;
    }

    [[nodiscard]] constexpr std::size_t hash() const noexcept
    {
        return std::hash<value_type>{}(value);
    }

    [[nodiscard]] constexpr auto operator<=>(tagged_id const &) const noexcept = default;

    [[nodiscard]] constexpr bool operator==(tagged_id const&) const noexcept = default;

    [[nodiscard]] constexpr bool operator==(nullptr_t) const noexcept
    {
        return value == invalid;
    }

    [[nodiscard]] constexpr bool operator==(std::nullopt_t) const noexcept
    {
        return value == invalid;
    }

    // clang-format off
    [[nodiscard]] constexpr bool operator==(signed char rhs) const noexcept { return value == rhs; }
    [[nodiscard]] constexpr bool operator==(signed short rhs) const noexcept { return value == rhs; }
    [[nodiscard]] constexpr bool operator==(signed int rhs) const noexcept { return value == rhs; }
    [[nodiscard]] constexpr bool operator==(signed long rhs) const noexcept { return value == rhs; }
    [[nodiscard]] constexpr bool operator==(signed long long rhs) const noexcept { return value == rhs; }
    [[nodiscard]] constexpr bool operator==(unsigned char rhs) const noexcept { return value == rhs; }
    [[nodiscard]] constexpr bool operator==(unsigned short rhs) const noexcept { return value == rhs; }
    [[nodiscard]] constexpr bool operator==(unsigned int rhs) const noexcept { return value == rhs; }
    [[nodiscard]] constexpr bool operator==(unsigned long rhs) const noexcept { return value == rhs; }
    [[nodiscard]] constexpr bool operator==(unsigned long long rhs) const noexcept { return value == rhs; }
    // clang-format on

    [[nodiscard]] bool holds_invariant() const noexcept
    {
        return value <= max or value == invalid;
    }

    [[nodiscard]] friend std::string to_string(tagged_id const& rhs) noexcept
    {
        return std::format("{}:{}", Tag, rhs.value);
    }

    friend std::ostream& operator<<(std::ostream& lhs, tagged_id const& rhs)
    {
        return lhs << to_string(rhs);
    }

private:
    value_type value;
};

} // namespace hi::inline v1

export template<typename T, hi::fixed_string Tag, std::size_t Max>
struct std::hash<hi::tagged_id<T, Tag, Max>> {
    [[nodiscard]] constexpr std::size_t operator()(hi::tagged_id<T, Tag, Max> const& rhs) const noexcept
    {
        return rhs.hash();
    }
};
