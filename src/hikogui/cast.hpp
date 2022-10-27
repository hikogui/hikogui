// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "utility.hpp"
#include "concepts.hpp"
#include "assert.hpp"
#include <type_traits>
#include <concepts>
#include <climits>

hi_warning_push();
// C26472: Don't use static_cast for arithmetic conversions, Use brace initialization, gsl::narrow_cast or gsl::narrow (type.1).
// This file implements narrow_cast().
hi_warning_ignore_msvc(26472);
// C26467: Converting from floating point to unsigned integral types results in non-portable code if the double/float has
// a negative value. Use gsl::narrow_cast or gsl::naroow instead to guard against undefined behavior and potential data loss
// (es.46).
// This file implements narrow_cast().
hi_warning_ignore_msvc(26467);
// C26496: The variable 'r' does not change after construction, mark it as const (con.4).
// False positive
hi_warning_ignore_msvc(26496);

namespace hi::inline v1 {
template<typename T>
[[nodiscard]] constexpr T copy(T value) noexcept
{
    return value;
}

/** Cast a pointer to a class to its base class or itself.
 */
template<typename Out, std::derived_from<std::remove_pointer_t<Out>> In>
[[nodiscard]] constexpr Out up_cast(In *rhs) noexcept
    requires std::is_pointer_v<Out> and (std::is_const_v<std::remove_pointer_t<Out>> == std::is_const_v<In> or std::is_const_v<std::remove_pointer_t<Out>>)
{
    return static_cast<Out>(rhs);
}

/** Cast a reference to a class to its base class or itself.
 */
template<typename Out>
[[nodiscard]] constexpr Out up_cast(nullptr_t) noexcept
    requires std::is_pointer_v<Out>
{
    return nullptr;
}

/** Cast a reference to a class to its base class or itself.
 */
template<typename Out, std::derived_from<std::remove_reference_t<Out>> In>
[[nodiscard]] constexpr Out up_cast(In& rhs) noexcept
    requires std::is_reference_v<Out> and (std::is_const_v<std::remove_reference_t<Out>> == std::is_const_v<In> or std::is_const_v<std::remove_reference_t<Out>>)
{
    return static_cast<Out>(rhs);
}

/** Cast a pointer to a class to its derived class or itself.
 *
 * @note It is undefined behavior if the argument is not of type Out.
 * @param rhs A pointer to an object that is of type `Out`. Or a nullptr which will be
 *        passed through.
 * @return A pointer to the same object with a new type.
 */
template<typename Out, base_of<std::remove_pointer_t<Out>> In>
[[nodiscard]] constexpr Out down_cast(In *rhs) noexcept
    requires std::is_pointer_v<Out> and (std::is_const_v<std::remove_pointer_t<Out>> == std::is_const_v<In> or std::is_const_v<std::remove_pointer_t<Out>>)
{
    hi_axiom(rhs == nullptr or dynamic_cast<Out>(rhs) != nullptr);
    return static_cast<Out>(rhs);
}

/** Cast a pointer to a class to its derived class or itself.
 *
 * @return A pointer to the same object with a new type.
 */
template<typename Out>
[[nodiscard]] constexpr Out down_cast(nullptr_t) noexcept
    requires std::is_pointer_v<Out>
{
    return nullptr;
}

/** Cast a reference to a class to its derived class or itself.
 *
 * @note It is undefined behavior if the argument is not of type Out.
 * @param rhs A reference to an object that is of type `Out`.
 * @return A reference to the same object with a new type.
 */
template<typename Out, base_of<std::remove_reference_t<Out>> In>
[[nodiscard]] constexpr Out down_cast(In& rhs) noexcept
    requires std::is_reference_v<Out> and (
    std::is_const_v<std::remove_reference_t<Out>> == std::is_const_v<In> or std::is_const_v<std::remove_reference_t<Out>>)
{
    hi_axiom(dynamic_cast<std::add_pointer_t<std::remove_reference_t<Out>>>(std::addressof(rhs)) != nullptr);
    return static_cast<Out>(rhs);
}

/** Cast a number to a type that will be able to represent all values without loss of precision.
 */
template<arithmetic Out, arithmetic In>
[[nodiscard]] constexpr Out wide_cast(In rhs) noexcept requires(type_in_range_v<Out, In>)
{
    return static_cast<Out>(rhs);
}

/** Cast a number to a type that will be able to represent all values without loss of precision.
 */
template<arithmetic Out>
[[nodiscard]] constexpr Out wide_cast(bool rhs) noexcept
{
    return static_cast<Out>(rhs);
}

namespace detail {

template<arithmetic Out, arithmetic In>
[[nodiscard]] constexpr bool narrow_validate(Out out, In in) noexcept
{
    // in- and out-value compares the same, after converting out-value back to in-type.
    auto r = (in == static_cast<In>(out));

    // If the types have different signs we need to do an extra test to make sure the actual sign
    // of the values are the same as well.
    if constexpr (std::numeric_limits<Out>::is_signed != std::numeric_limits<In>::is_signed) {
        r &= (in < In{}) == (out < Out{});
    }

    return r;
}

} // namespace detail

/** Cast an unsigned number and saturate on overflow.
 */
template<std::unsigned_integral Out, std::unsigned_integral In>
[[nodiscard]] constexpr Out saturate_cast(In rhs) noexcept
{
    auto r = std::numeric_limits<Out>::max();
    if (rhs < r) {
        r = static_cast<Out>(rhs);
    }
    return r;
}

/** Cast numeric values without loss of precision.
 *
 * @note It is undefined behavior to cast a value which will cause a loss of precision.
 * @tparam Out The numeric type to cast to
 * @tparam In The numeric type to cast from
 * @param rhs The value to cast.
 * @return The value casted to a different type without loss of precision.
 */
template<arithmetic Out, arithmetic In>
[[nodiscard]] constexpr Out narrow_cast(In rhs) noexcept
{
    if constexpr (type_in_range_v<Out, In>) {
        return static_cast<Out>(rhs);
    } else {
        hilet r = static_cast<Out>(rhs);
        hi_axiom(detail::narrow_validate(r, rhs));
        return r;
    }
}

template<std::integral Out, arithmetic In>
[[nodiscard]] constexpr Out truncate(In rhs) noexcept
{
    return static_cast<Out>(rhs);
}

/** Cast a character.
 *
 * Both the input and output types are interpreted as unsigned values, even if
 * they are signed values. For example `char` may be either signed or unsigned,
 * but you have to treat those as unsigned values.
 * 
 * @note @a rhs value after casting, must fit in the output type.
 * @param rhs The value of the character.
 * @return The casted value.
 */
template<std::integral Out, std::integral In>
[[nodiscard]] constexpr Out char_cast(In rhs) noexcept
{
    using in_unsigned_type = std::make_unsigned_t<In>;
    using out_unsigned_type = std::make_unsigned_t<Out>;

    // We cast to unsigned of the same type, so that we don't accidentally sign extent 'char'.
    auto in_unsigned = static_cast<in_unsigned_type>(rhs);
    auto out_unsigned = narrow_cast<out_unsigned_type>(in_unsigned);
    return static_cast<Out>(out_unsigned);
}

template<std::integral Out>
[[nodiscard]] constexpr Out char_cast(std::byte rhs) noexcept
{
    return char_cast<Out>(static_cast<uint8_t>(rhs));
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

template<typename T>
[[nodiscard]] constexpr bool to_bool(T&& rhs) noexcept requires(requires(T&& x) { static_cast<bool>(std::forward<T>(x)); })
{
    return static_cast<bool>(std::forward<T>(rhs));
}

} // namespace hi::inline v1

hi_warning_pop();
