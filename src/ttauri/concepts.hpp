// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "type_traits.hpp"
#include <type_traits>
#include <string>
#include <concepts>
#include <limits>
#include <coroutine>

namespace tt::inline v1 {

template<typename T>
concept numeric_limited = std::numeric_limits<T>::is_specialized;

template<typename T>
concept numeric = is_numeric_v<T>;

template<typename T>
concept numeric_integral = is_numeric_integral_v<T>;

template<typename T>
concept numeric_signed_integral = is_numeric_signed_integral_v<T>;

template<typename T>
concept numeric_unsigned_integral = is_numeric_unsigned_integral_v<T>;

template<typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template<typename T>
concept pointer = std::is_pointer_v<T>;

template<typename T>
concept reference = std::is_reference_v<T>;

template<typename T>
concept lvalue_reference = std::is_lvalue_reference_v<T>;

template<typename T>
concept rvalue_reference = std::is_rvalue_reference_v<T>;

template<typename T>
concept trivially_copyable = std::is_trivially_copyable_t<T>;

template<typename BaseType, typename DerivedType>
concept base_of = std::is_base_of_v<BaseType, DerivedType>;

template<typename BaseType, typename DerivedType>
concept decayed_base_of = is_decayed_base_of_v<BaseType, DerivedType>;

template<typename DerivedType, typename BaseType>
concept derived_from = tt::is_derived_from_v<DerivedType, BaseType>;

template<typename DerivedType, typename BaseType>
concept decayed_derived_from = tt::is_decayed_derived_from_v<DerivedType, BaseType>;

template<typename BaseType, typename DerivedType>
concept strict_base_of = base_of<BaseType, DerivedType> && !std::same_as<BaseType, DerivedType>;

template<typename BaseType, typename DerivedType>
concept strict_derived_from = derived_from<BaseType, DerivedType> && !std::same_as<BaseType, DerivedType>;

template<typename T>
concept pre_incrementable = requires(T a)
{
    {++a};
};

template<typename T>
concept pre_decrementable = requires(T a)
{
    {--a};
};

template<typename T>
concept to_stringable = requires(T v)
{
    {
        to_string(v)
        } -> std::convertible_to<std::string>;
};

template<typename T>
concept from_stringable = requires()
{
    {
        from_string<T>(std::string_view{})
        } -> std::convertible_to<T>;
};

template<typename From, typename To>
concept static_castableable = requires(From v)
{
    {
        static_cast<To>(v)
        } -> std::convertible_to<To>;
};

template<typename T>
concept sizeable = requires(T v)
{
    {
        size(v)
        } -> std::convertible_to<std::size_t>;
};

template<typename T>
concept scalar = std::is_scalar_v<T>;

/** Concept for std::is_scoped_enum_v<T>.
 *
 * XXX std::is_scoped_enum_v<T> ios a c++23 feature, so right now we use std::is_enum_v<T> instead.
 */
template<typename T>
concept scoped_enum = std::is_enum_v<T>;

template<typename Forward, typename T>
concept forward_of = is_forward_of_v<T, Forward>;


/** Check if type can be directly co_await on.
 *
 * The type needs to have the following member functions:
 *  `await_ready()`, `await_suspend()` and `await_resume()`.
 */
template<typename T>
concept awaitable_direct = requires(T a, std::coroutine_handle<> b)
{
    // clang-format off
    { a.await_ready() } -> std::convertible_to<bool>;
    a.await_suspend(b);
    a.await_resume();
    // clang-format on
};

/** Check if type can be indirectly co_await on.
 *
 * The type needs to implement member function `operator co_await()`.
 */
template<typename T>
concept awaitable_member = requires(T a)
{
    {a.operator co_await()};
};

/** Check if type can be indirectly co_await on.
 *
 * The type needs to implement free function `operator co_await()`.
 */
template<typename T>
concept awaitable_non_member = requires(T a)
{
    {operator co_await(static_cast<T &&>(a))};
};

/** Check if type can be directly or indirectly co_await on.
 *
 * The type needs to have the following member functions:
 *  `await_ready()`, `await_suspend()` and `await_resume()`.
 * Or implement `operator co_await()`.
 */
template<typename T>
concept awaitable = awaitable_direct<T> or awaitable_member<T> or awaitable_non_member<T>;


} // namespace tt::inline v1
