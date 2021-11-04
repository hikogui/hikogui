// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include "cast.hpp"
#include "math.hpp"
#include "fixed_string.hpp"
#include <limits>
#include <typeinfo>
#include <typeindex>

namespace tt::inline v1 {

template<typename T, basic_fixed_string Tag, ssize_t Max = std::numeric_limits<T>::max() - 1>
class tagged_id {
public:
    static_assert(std::is_integral_v<T>, "Expecting tagged_id to be an integral");
    static_assert(std::is_unsigned_v<T>, "Expecting tagged_id to be an unsigned integer");
    static_assert(Max < std::numeric_limits<T>::max(), "Max must be at least one less than the maximum value of T");

    using value_type = T;

    constexpr static value_type max = Max;
    constexpr static value_type invalid = max + 1;
    constexpr static value_type mask = static_cast<value_type>((1ULL << std::bit_width(invalid)) - 1);

    constexpr explicit tagged_id(signed long long rhs) noexcept : value(narrow_cast<value_type>(rhs))
    {
        tt_axiom(value <= invalid);
    }
    constexpr explicit tagged_id(signed long rhs) noexcept : value(narrow_cast<value_type>(rhs))
    {
        tt_axiom(value <= invalid);
    }
    constexpr explicit tagged_id(signed int rhs) noexcept : value(narrow_cast<value_type>(rhs))
    {
        tt_axiom(value <= invalid);
    }
    constexpr explicit tagged_id(signed short rhs) noexcept : value(narrow_cast<value_type>(rhs))
    {
        tt_axiom(value <= invalid);
    }
    constexpr explicit tagged_id(signed char rhs) noexcept : value(narrow_cast<value_type>(rhs))
    {
        tt_axiom(value <= invalid);
    }
    constexpr explicit tagged_id(unsigned long long rhs) noexcept : value(narrow_cast<value_type>(rhs))
    {
        tt_axiom(value <= invalid);
    }
    constexpr explicit tagged_id(unsigned long rhs) noexcept : value(narrow_cast<value_type>(rhs))
    {
        tt_axiom(value <= invalid);
    }
    constexpr explicit tagged_id(unsigned int rhs) noexcept : value(narrow_cast<value_type>(rhs))
    {
        tt_axiom(value <= invalid);
    }
    constexpr explicit tagged_id(unsigned short rhs) noexcept : value(narrow_cast<value_type>(rhs))
    {
        tt_axiom(value <= invalid);
    }
    constexpr explicit tagged_id(unsigned char rhs) noexcept : value(narrow_cast<value_type>(rhs))
    {
        tt_axiom(value <= invalid);
    }

    constexpr tagged_id &operator=(signed long long rhs) noexcept
    {
        value = narrow_cast<value_type>(rhs);
        tt_axiom(value <= invalid);
        return *this;
    }
    constexpr tagged_id &operator=(signed long rhs) noexcept
    {
        value = narrow_cast<value_type>(rhs);
        tt_axiom(value <= invalid);
        return *this;
    }
    constexpr tagged_id &operator=(signed int rhs) noexcept
    {
        value = narrow_cast<value_type>(rhs);
        tt_axiom(value <= invalid);
        return *this;
    }
    constexpr tagged_id &operator=(signed short rhs) noexcept
    {
        value = narrow_cast<value_type>(rhs);
        tt_axiom(value <= invalid);
        return *this;
    }
    constexpr tagged_id &operator=(signed char rhs) noexcept
    {
        value = narrow_cast<value_type>(rhs);
        tt_axiom(value <= invalid);
        return *this;
    }
    constexpr tagged_id &operator=(unsigned long long rhs) noexcept
    {
        value = narrow_cast<value_type>(rhs);
        tt_axiom(value <= invalid);
        return *this;
    }
    constexpr tagged_id &operator=(unsigned long rhs) noexcept
    {
        value = narrow_cast<value_type>(rhs);
        tt_axiom(value <= invalid);
        return *this;
    }
    constexpr tagged_id &operator=(unsigned int rhs) noexcept
    {
        value = narrow_cast<value_type>(rhs);
        tt_axiom(value <= invalid);
        return *this;
    }
    constexpr tagged_id &operator=(unsigned short rhs) noexcept
    {
        value = narrow_cast<value_type>(rhs);
        tt_axiom(value <= invalid);
        return *this;
    }
    constexpr tagged_id &operator=(unsigned char rhs) noexcept
    {
        value = narrow_cast<value_type>(rhs);
        tt_axiom(value <= invalid);
        return *this;
    }

    constexpr tagged_id() noexcept : value(invalid) {}
    constexpr tagged_id(tagged_id const &other) noexcept = default;
    constexpr tagged_id(tagged_id &&other) noexcept = default;
    constexpr tagged_id &operator=(tagged_id const &other) noexcept = default;
    constexpr tagged_id &operator=(tagged_id &&other) noexcept = default;

    constexpr operator signed long long() const noexcept
    {
        return narrow_cast<signed long long>(value);
    }
    constexpr operator signed long() const noexcept
    {
        return narrow_cast<signed long>(value);
    }
    constexpr operator signed int() const noexcept
    {
        return narrow_cast<signed int>(value);
    }
    constexpr operator signed short() const noexcept
    {
        return narrow_cast<signed short>(value);
    }
    constexpr operator signed char() const noexcept
    {
        return narrow_cast<signed char>(value);
    }
    constexpr operator unsigned long long() const noexcept
    {
        return narrow_cast<unsigned long long>(value);
    }
    constexpr operator unsigned long() const noexcept
    {
        return narrow_cast<unsigned long>(value);
    }
    constexpr operator unsigned int() const noexcept
    {
        return narrow_cast<unsigned int>(value);
    }
    constexpr operator unsigned short() const noexcept
    {
        return narrow_cast<unsigned short>(value);
    }
    constexpr operator unsigned char() const noexcept
    {
        return narrow_cast<unsigned char>(value);
    }

    constexpr operator bool() const noexcept
    {
        return value <= max;
    }

    [[nodiscard]] constexpr size_t hash() const noexcept
    {
        return std::hash<value_type>{}(value);
    }

    [[nodiscard]] constexpr friend bool operator==(tagged_id const &lhs, tagged_id const &rhs) noexcept
    {
        return lhs.value == rhs.value;
    }
    [[nodiscard]] constexpr friend bool operator!=(tagged_id const &lhs, tagged_id const &rhs) noexcept
    {
        return lhs.value != rhs.value;
    }
    [[nodiscard]] constexpr friend bool operator<(tagged_id const &lhs, tagged_id const &rhs) noexcept
    {
        return lhs.value < rhs.value;
    }
    [[nodiscard]] constexpr friend bool operator>(tagged_id const &lhs, tagged_id const &rhs) noexcept
    {
        return lhs.value > rhs.value;
    }
    [[nodiscard]] constexpr friend bool operator<=(tagged_id const &lhs, tagged_id const &rhs) noexcept
    {
        return lhs.value <= rhs.value;
    }
    [[nodiscard]] constexpr friend bool operator>=(tagged_id const &lhs, tagged_id const &rhs) noexcept
    {
        return lhs.value >= rhs.value;
    }

    template<typename O>
    [[nodiscard]] constexpr friend bool operator==(tagged_id const &lhs, O const &rhs) noexcept
    {
        return lhs == tagged_id{rhs};
    }
    template<typename O>
    [[nodiscard]] constexpr friend bool operator!=(tagged_id const &lhs, O const &rhs) noexcept
    {
        return lhs != tagged_id{rhs};
    }
    template<typename O>
    [[nodiscard]] constexpr friend bool operator<(tagged_id const &lhs, O const &rhs) noexcept
    {
        return lhs < tagged_id{rhs};
    }
    template<typename O>
    [[nodiscard]] constexpr friend bool operator>(tagged_id const &lhs, O const &rhs) noexcept
    {
        return lhs > tagged_id{rhs};
    }
    template<typename O>
    [[nodiscard]] constexpr friend bool operator<=(tagged_id const &lhs, O const &rhs) noexcept
    {
        return lhs <= tagged_id{rhs};
    }
    template<typename O>
    [[nodiscard]] constexpr friend bool operator>=(tagged_id const &lhs, O const &rhs) noexcept
    {
        return lhs >= tagged_id{rhs};
    }
    template<typename O>
    [[nodiscard]] constexpr friend bool operator==(O const &lhs, tagged_id const &rhs) noexcept
    {
        return tagged_id{lhs} == rhs;
    }
    template<typename O>
    [[nodiscard]] constexpr friend bool operator!=(O const &lhs, tagged_id const &rhs) noexcept
    {
        return tagged_id{lhs} != rhs;
    }
    template<typename O>
    [[nodiscard]] constexpr friend bool operator<(O const &lhs, tagged_id const &rhs) noexcept
    {
        return tagged_id{lhs} < rhs;
    }
    template<typename O>
    [[nodiscard]] constexpr friend bool operator>(O const &lhs, tagged_id const &rhs) noexcept
    {
        return tagged_id{lhs} > rhs;
    }
    template<typename O>
    [[nodiscard]] constexpr friend bool operator<=(O const &lhs, tagged_id const &rhs) noexcept
    {
        return tagged_id{lhs} <= rhs;
    }
    template<typename O>
    [[nodiscard]] constexpr friend bool operator>=(O const &lhs, tagged_id const &rhs) noexcept
    {
        return tagged_id{lhs} >= rhs;
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

} // namespace tt

namespace std {

template<typename T, tt::basic_fixed_string Tag, tt::ssize_t Max>
struct hash<tt::tagged_id<T, Tag, Max>> {
    [[nodiscard]] constexpr size_t operator()(tt::tagged_id<T, Tag, Max> const &rhs) const noexcept
    {
        return rhs.hash();
    }
};

} // namespace std
