// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include <cstdint>

namespace tt {

enum class unicode_general_category : uint8_t {
    unknown,
    Lu,
    Ll,
    Lt,
    Lm,
    Lo,
    Mn,
    Mc,
    Me,
    Nd,
    Nl,
    No,
    Pc,
    Pd,
    Ps,
    Pe,
    Pi,
    Pf,
    Po,
    Sm,
    Sc,
    Sk,
    So,
    Zs,
    Zl,
    Zp,
    Cc,
    Cf,
    Cs,
    Co,
    Cn
};

[[nodiscard]] constexpr bool is_LC(unicode_general_category const &rhs) noexcept
{
    using enum unicode_general_category;
    return rhs == Lu || rhs == Ll || rhs == Lt;
}

[[nodiscard]] constexpr bool is_L(unicode_general_category const &rhs) noexcept
{
    using enum unicode_general_category;
    return is_LC(rhs) || rhs == Lm || rhs == Lo;
}

[[nodiscard]] constexpr bool is_M(unicode_general_category const &rhs) noexcept
{
    using enum unicode_general_category;
    return rhs == Mn || rhs == Mc || rhs == Me;
}

[[nodiscard]] constexpr bool is_N(unicode_general_category const &rhs) noexcept
{
    using enum unicode_general_category;
    return rhs == Nd || rhs == Nl || rhs == No;
}

[[nodiscard]] constexpr bool is_P(unicode_general_category const &rhs) noexcept
{
    using enum unicode_general_category;
    return rhs == Pc || rhs == Pd || rhs == Ps || rhs == Pe || rhs == Pi || rhs == Pf || rhs == Po;
}

[[nodiscard]] constexpr bool is_S(unicode_general_category const &rhs) noexcept
{
    using enum unicode_general_category;
    return rhs == Sm || rhs == Sc || rhs == Sk || rhs == So;
}

[[nodiscard]] constexpr bool is_Z(unicode_general_category const &rhs) noexcept
{
    using enum unicode_general_category;
    return rhs == Zs || rhs == Zl || rhs == Zp;
}

[[nodiscard]] constexpr bool is_C(unicode_general_category const &rhs) noexcept
{
    using enum unicode_general_category;
    return rhs == Cc || rhs == Cf || rhs == Cs || rhs == Co || rhs == Cn;
}

[[nodiscard]] constexpr bool is_visible(unicode_general_category const &rhs) noexcept
{
    return is_L(rhs) | is_M(rhs) | is_N(rhs) | is_P(rhs) | is_S(rhs);
}

} // namespace tt
