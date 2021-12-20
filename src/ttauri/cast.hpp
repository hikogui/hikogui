// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include "concepts.hpp"
#include "assert.hpp"
#include <type_traits>
#include <concepts>
#include <climits>

namespace tt::inline v1 {

template<typename T>
[[nodiscard]] constexpr T copy(T value) noexcept
{
    return value;
}


/** Up-cast executes an always safe cast between types.
 */
template<std::integral Out, std::integral In>
[[nodiscard]] constexpr Out up_cast(In rhs) noexcept
{
    static_assert(type_in_range<Out,In>, "invalid up_cast() integer argument may be out of range of integer return type.");
    return static_cast<Out>(rhs);
}

/** Up-cast executes an always safe cast between types.
 */
template<std::floating_point Out, std::floating_point In>
[[nodiscard]] constexpr Out up_cast(In rhs) noexcept
{
    static_assert(sizeof(Out) >= sizeof(In), "invalid up_cast() float argument larger than float return type.");
    return static_cast<Out>(rhs);
}

/** Up-cast executes an always safe cast between types.
 */
template<std::floating_point Out, std::integral In>
[[nodiscard]] constexpr Out up_cast(In rhs) noexcept
{
    return static_cast<Out>(rhs);
}

/** Up-cast executes an always safe cast between types.
 */
template<typename Out, typename In>
[[nodiscard]] constexpr Out *up_cast(In *rhs) noexcept
{
    static_assert(std::is_pointer_interconvertible_base_of<Out,In>, "invalid up_cast() argument type is not derived from return type");
    static_assert(std::is_const_v<Out> == std::is_const_v<in> or std::is_const_v<Out>, "invalid up_cast() argument is const return type is non-const");
    return static_cast<Out *>(rhs);
}

/** Up-cast executes an always safe cast between types.
 */
template<typename Out, typename In>
[[nodiscard]] constexpr Out &up_cast(In &rhs) noexcept
{
    static_assert(std::is_pointer_interconvertible_base_of<Out,In>, "invalid up_cast() argument type is not derived from return type");
    static_assert(std::is_const_v<Out> == std::is_const_v<in> or std::is_const_v<Out>, "invalid up_cast() argument is const return type is non-const");
    return static_cast<Out &>(rhs);
}


template<std::signed_integral OutType, std::floating_point InType>
[[nodiscard]] constexpr OutType narrow_cast(InType value) noexcept
{
    tt_axiom(value >= std::numeric_limits<OutType>::lowest() && value <= std::numeric_limits<OutType>::max());
    return static_cast<OutType>(value);
}

template<std::signed_integral OutType, std::signed_integral InType>
[[nodiscard]] constexpr OutType narrow_cast(InType value) noexcept
{
    constexpr auto smin = static_cast<long long>(std::numeric_limits<OutType>::lowest());
    constexpr auto smax = static_cast<long long>(std::numeric_limits<OutType>::max());
    tt_axiom(value >= smin && value <= smax);
    return static_cast<OutType>(value);
}

template<std::signed_integral OutType, std::unsigned_integral InType>
[[nodiscard]] constexpr OutType narrow_cast(InType value) noexcept
{
    constexpr auto umax = static_cast<unsigned long long>(std::numeric_limits<OutType>::max());
    tt_axiom(value <= umax);
    return static_cast<OutType>(value);
}

template<std::unsigned_integral OutType, std::floating_point InType>
[[nodiscard]] constexpr OutType narrow_cast(InType value) noexcept
{
    tt_axiom(value >= InType{0});
    tt_axiom(value <= std::numeric_limits<OutType>::max());
    return static_cast<OutType>(value);
}

template<std::unsigned_integral OutType, std::signed_integral InType>
[[nodiscard]] constexpr OutType narrow_cast(InType value) noexcept
{
    tt_axiom(value >= InType{0});
    if constexpr (sizeof(OutType) < sizeof(InType)) {
        constexpr auto smax = static_cast<long long>(std::numeric_limits<OutType>::max());
        tt_axiom(value <= smax);
    }
    return static_cast<OutType>(value);
}

template<std::unsigned_integral OutType, std::unsigned_integral InType>
[[nodiscard]] constexpr OutType narrow_cast(InType value) noexcept
{
    constexpr auto umax = static_cast<unsigned long long>(std::numeric_limits<OutType>::max());
    tt_axiom(value <= umax);
    return static_cast<OutType>(value);
}

template<std::floating_point OutType, tt::arithmetic InType>
[[nodiscard]] constexpr OutType narrow_cast(InType value) noexcept
{
    return static_cast<OutType>(value);
}

template<tt::lvalue_reference BaseType, tt::derived_from<std::remove_reference_t<BaseType>> DerivedType>
[[nodiscard]] constexpr BaseType narrow_cast(DerivedType &value) noexcept
{
    static_assert(
        !std::is_const_v<DerivedType> || std::is_const_v<std::remove_reference_t<BaseType>>,
        "narrow_cast must not cast away const");
    return static_cast<BaseType>(value);
}

template<tt::lvalue_reference DerivedType, tt::strict_base_of<std::remove_reference_t<DerivedType>> BaseType>
[[nodiscard]] constexpr DerivedType narrow_cast(BaseType &value) noexcept
{
    static_assert(
        !std::is_const_v<BaseType> || std::is_const_v<std::remove_reference_t<DerivedType>>,
        "narrow_cast must not cast away const");
    tt_axiom(dynamic_cast<std::remove_reference_t<DerivedType> *>(&value) != nullptr);
    return static_cast<DerivedType>(value);
}

template<tt::pointer BaseType, tt::derived_from<std::remove_pointer_t<BaseType>> DerivedType>
[[nodiscard]] constexpr BaseType narrow_cast(DerivedType *value) noexcept
{
    static_assert(
        !std::is_const_v<DerivedType> || std::is_const_v<std::remove_pointer_t<BaseType>>,
        "narrow_cast must not cast away const");
    return static_cast<BaseType>(value);
}

template<tt::pointer DerivedType, tt::strict_base_of<std::remove_pointer_t<DerivedType>> BaseType>
[[nodiscard]] constexpr DerivedType narrow_cast(BaseType *value) noexcept
{
    static_assert(
        !std::is_const_v<BaseType> || std::is_const_v<std::remove_pointer_t<DerivedType>>,
        "narrow_cast must not cast away const");
    tt_axiom(value == nullptr or dynamic_cast<DerivedType>(value) != nullptr);
    return static_cast<DerivedType>(value);
}

/** Return the low half of the input value.
 */
template<std::unsigned_integral OutType, std::unsigned_integral InType>
[[nodiscard]] constexpr OutType low_bit_cast(InType value) noexcept
{
    static_assert(sizeof(OutType) * 2 == sizeof(InType), "Return value of low_bit_cast must be half the size of the input");
    return static_cast<OutType>(value);
}

/** Return the upper half of the input value.
 */
template<std::unsigned_integral OutType, std::unsigned_integral InType>
[[nodiscard]] constexpr OutType high_bit_cast(InType value) noexcept
{
    static_assert(sizeof(OutType) * 2 == sizeof(InType), "Return value of high_bit_cast must be half the size of the input");
    return static_cast<OutType>(value >> sizeof(OutType) * CHAR_BIT);
}

/** Return the low half of the input value.
 */
template<std::signed_integral OutType, std::signed_integral InType>
[[nodiscard]] constexpr OutType low_bit_cast(InType value) noexcept
{
    using UInType = std::make_unsigned_t<InType>;
    using UOutType = std::make_unsigned_t<OutType>;
    return static_cast<OutType>(low_bit_cast<UOutType>(static_cast<UInType>(value)));
}

/** Return the upper half of the input value.
 */
template<std::signed_integral OutType, std::signed_integral InType>
[[nodiscard]] constexpr OutType high_bit_cast(InType value) noexcept
{
    using UInType = std::make_unsigned_t<InType>;
    using UOutType = std::make_unsigned_t<OutType>;
    return static_cast<OutType>(high_bit_cast<UOutType>(static_cast<UInType>(value)));
}

/** Return the low half of the input value.
 */
template<std::unsigned_integral OutType, std::signed_integral InType>
[[nodiscard]] constexpr OutType low_bit_cast(InType value) noexcept
{
    using UInType = std::make_unsigned_t<InType>;
    return low_bit_cast<OutType>(static_cast<UInType>(value));
}

/** Return the upper half of the input value.
 */
template<std::unsigned_integral OutType, std::signed_integral InType>
[[nodiscard]] constexpr OutType high_bit_cast(InType value) noexcept
{
    using UInType = std::make_unsigned_t<InType>;
    return high_bit_cast<OutType>(static_cast<UInType>(value));
}

/** Return the upper half of the input value.
 */
template<std::unsigned_integral OutType, std::unsigned_integral InType>
[[nodiscard]] constexpr OutType merge_bit_cast(InType hi, InType lo) noexcept
{
    static_assert(sizeof(OutType) == sizeof(InType) * 2, "Return value of merge_bit_cast must be double the size of the input");

    OutType r = static_cast<OutType>(hi);
    r <<= sizeof(InType) * CHAR_BIT;
    r |= static_cast<OutType>(lo);
    return r;
}

/** Return the upper half of the input value.
 */
template<std::signed_integral OutType, std::signed_integral InType>
[[nodiscard]] constexpr OutType merge_bit_cast(InType hi, InType lo) noexcept
{
    using UInType = std::make_unsigned_t<InType>;
    using UOutType = std::make_unsigned_t<OutType>;
    return static_cast<OutType>(merge_bit_cast<UOutType>(static_cast<UInType>(hi), static_cast<UInType>(lo)));
}

/** Return the upper half of the input value.
 */
template<std::signed_integral OutType, std::unsigned_integral InType>
[[nodiscard]] constexpr OutType merge_bit_cast(InType hi, InType lo) noexcept
{
    using UOutType = std::make_unsigned_t<OutType>;
    return narrow_cast<OutType>(merge_bit_cast<UOutType>(hi, lo));
}

[[nodiscard]] constexpr auto to_underlying(scoped_enum auto rhs) noexcept
{
    return static_cast<std::underlying_type_t<decltype(rhs)>>(rhs);
}

} // namespace tt::inline v1
