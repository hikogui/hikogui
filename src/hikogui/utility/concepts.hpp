// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "type_traits.hpp"
#include "../macros.hpp"
#include <type_traits>
#include <string>
#include <concepts>
#include <limits>
#include <coroutine>
#include <chrono>

hi_export_module(hikogui.utility.concepts);

hi_export namespace hi { inline namespace v1 {

hi_export template<typename T>
concept numeric = is_numeric_v<T>;

hi_export template<typename T>
concept numeric_integral = is_numeric_integral_v<T>;

hi_export template<typename T>
concept numeric_signed_integral = is_numeric_signed_integral_v<T>;

hi_export template<typename T>
concept numeric_unsigned_integral = is_numeric_unsigned_integral_v<T>;

hi_export template<typename T>
concept arithmetic = std::is_arithmetic_v<T>;

hi_export template<typename T>
concept lvalue_reference = std::is_lvalue_reference_v<T>;

hi_export template<typename T>
concept rvalue_reference = std::is_rvalue_reference_v<T>;

hi_export template<typename T>
concept trivially_copyable = std::is_trivially_copyable_v<T>;

/** Different from
 * 
 * Not std::same_as.
 */
hi_export template<typename Context, typename Expected>
concept different_from = not std::same_as<Context, Expected>;

/** Incompatible with another type
 * 
 * Not std::convertible_to.
 */
hi_export template<typename Context, typename Expected>
concept incompatible_with = not std::convertible_to<Context, Expected>;

hi_export template<typename BaseType, typename DerivedType>
concept base_of = std::is_base_of_v<BaseType, DerivedType>;

hi_export template<typename BaseType, typename DerivedType>
concept decayed_base_of = is_decayed_base_of_v<BaseType, DerivedType>;

hi_export template<typename Context, typename Expected>
concept derived_from = hi::is_derived_from_v<Context, Expected>;

hi_export template<typename DerivedType, typename BaseType>
concept decayed_derived_from = hi::is_decayed_derived_from_v<DerivedType, BaseType>;

hi_export template<typename BaseType, typename DerivedType>
concept strict_base_of = base_of<BaseType, DerivedType> && !std::same_as<BaseType, DerivedType>;

hi_export template<typename BaseType, typename DerivedType>
concept strict_derived_from = derived_from<BaseType, DerivedType> && !std::same_as<BaseType, DerivedType>;

hi_export template<typename T>
concept pre_incrementable = requires(T a) {
    {
        ++a
    };
};

hi_export template<typename T>
concept pre_decrementable = requires(T a) {
    {
        --a
    };
};

hi_export template<typename T>
concept to_stringable = requires(T v) {
    {
        to_string(v)
    } -> std::convertible_to<std::string>;
};

hi_export template<typename T>
concept from_stringable = requires() {
    {
        from_string<T>(std::string_view{})
    } -> std::convertible_to<T>;
};

hi_export template<typename From, typename To>
concept static_castableable = requires(From v) {
    {
        static_cast<To>(v)
    } -> std::convertible_to<To>;
};

hi_export template<typename T>
concept sizeable = requires(T v) {
    {
        size(v)
    } -> std::convertible_to<std::size_t>;
};

hi_export template<typename T>
concept scalar = std::is_scalar_v<T>;

/** Concept for std::is_scoped_enum_v<T>.
 *
 * XXX std::is_scoped_enum_v<T> ios a c++23 feature, so right now we use std::is_enum_v<T> instead.
 */
hi_export template<typename T>
concept scoped_enum = std::is_enum_v<T>;

/** True if T is a forwarded type of Forward.
 *
 * @see is_forward_of
 */
hi_export template<typename Context, typename Expected, typename... OtherExpected>
concept forward_of = is_forward_of_v<Context, Expected, OtherExpected...>;

/** An array of this type will implicitly create objects within that array.
 *
 * P059R6: Implicit creation of objects for low-level object manipulation.
 */
hi_export template<typename Context>
concept byte_like = is_byte_like_v<Context>;

/** True if T can be assigned with a nullptr.
 */
hi_export template<typename T>
concept nullable = requires (T &a) { a = nullptr; };

/** True if T is dereferenceable.
 * 
 * Either it is a pointer, or it implements both operator*() and operator->().
 */
hi_export template<typename T>
concept dereferenceable = std::is_pointer_v<T> or requires (T &a)
{
    a.operator*();
    a.operator->();
};

/** True if T is both nullable and dereferenceable.
 */
hi_export template<typename T>
concept nullable_pointer = nullable<T> and dereferenceable<T>;

}} // namespace hi::v1
