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
    requires(std::is_const_v<std::remove_pointer_t<Out>> == std::is_const_v<In> or std::is_const_v<std::remove_pointer_t<Out>>)
{
    return static_cast<Out>(rhs);
}

/** Cast a reference to a class to its base class or itself.
 */
template<typename Out>
[[nodiscard]] constexpr Out up_cast(nullptr_t) noexcept
{
    return nullptr;
}

/** Cast a reference to a class to its base class or itself.
 */
template<typename Out, std::derived_from<std::remove_reference_t<Out>> In>
[[nodiscard]] constexpr Out up_cast(In &rhs) noexcept requires(
    std::is_const_v<std::remove_reference_t<Out>> == std::is_const_v<In> or std::is_const_v<std::remove_reference_t<Out>>)
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
    requires(std::is_const_v<std::remove_pointer_t<Out>> == std::is_const_v<In> or std::is_const_v<std::remove_pointer_t<Out>>)
{
    hi_axiom(rhs == nullptr or dynamic_cast<Out>(rhs) != nullptr);
    return static_cast<Out>(rhs);
}

/** Cast a pointer to a class to its derived class or itself.
 *
 * @note It is undefined behavior if the argument is not of type Out.
 * @param rhs A pointer to an object that is of type `Out`. Or a nullptr which will be
 *        passed through.
 * @return A pointer to the same object with a new type.
 */
template<typename Out>
[[nodiscard]] constexpr Out down_cast(nullptr_t) noexcept
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
[[nodiscard]] constexpr Out down_cast(In &rhs) noexcept requires(
    std::is_const_v<std::remove_reference_t<Out>> == std::is_const_v<In> or std::is_const_v<std::remove_reference_t<Out>>)
{
    return static_cast<Out>(rhs);
}

/** Cast a number to a type that will be able to represent all values without loss of precision.
 */
template<numeric Out, numeric In>
[[nodiscard]] constexpr Out wide_cast(In rhs) noexcept requires(type_in_range_v<Out, In>)
{
    return static_cast<Out>(rhs);
}

namespace detail {

template<numeric Out, numeric In>
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

}

/** Cast numeric values without loss of precision.
 *
 * @tparam Out The numeric type to cast to
 * @tparam In The numeric type to cast from
 * @param rhs The value to cast.
 * @return The value casted to a different type without loss of precision.
 * @throws std::bad_cast when the value could not be casted without loss of precision.
 */
template<numeric Out, numeric In>
[[nodiscard]] constexpr Out narrow(In rhs) noexcept(type_in_range_v<Out, In>)
{
    if constexpr (type_in_range_v<Out, In>) {
        return static_cast<Out>(rhs);
    } else {
        hilet r = static_cast<Out>(rhs);

        if (not detail::narrow_validate(r, rhs)) {
            throw std::bad_cast();
        }

        return r;
    }
}

/** Cast numeric values without loss of precision.
 *
 * @note It is undefined behavior to cast a value which will cause a loss of precision.
 * @tparam Out The numeric type to cast to
 * @tparam In The numeric type to cast from
 * @param rhs The value to cast.
 * @return The value casted to a different type without loss of precision.
 */
template<numeric Out, numeric In>
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

/** Cast a object to an directly-awaitable object.
 *
 * This function may use `operator co_await()` to retrieve the actual awaitable.
 */
decltype(auto) awaitable_cast(awaitable_direct auto &&rhs) noexcept
{
    return hi_forward(rhs);
}

/** Cast a object to an directly-awaitable object.
 *
 * This function may use `operator co_await()` to retrieve the actual awaitable.
 */
decltype(auto) awaitable_cast(awaitable_member auto &&rhs) noexcept
{
    return hi_forward(rhs).operator co_await();
}

/** Cast a object to an directly-awaitable object.
 *
 * This function may use `operator co_await()` to retrieve the actual awaitable.
 */
decltype(auto) awaitable_cast(awaitable_non_member auto &&rhs) noexcept
{
    return operator co_await(hi_forward(rhs));
}

/** Resolve the type that is directly-awaitable.
 *
 * This function may use `operator co_await()` to retrieve the actual awaitable type.
 */
template<awaitable T>
struct awaitable_cast_type {
    using type = std::remove_cvref_t<decltype(awaitable_cast(std::declval<T>()))>;
};

/** Resolve the type that is directly-awaitable.
 *
 * This function may use `operator co_await()` to retrieve the actual awaitable type.
 */
template<awaitable T>
using awaitable_cast_type_t = awaitable_cast_type<T>::type;



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

} // namespace hi::inline v1
