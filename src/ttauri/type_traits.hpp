// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file
 */

#pragma once

#include "architecture.hpp"
#include <cstdint>
#include <type_traits>
#include <string>
#include <string_view>
#include <memory>

namespace tt::inline v1 {

// clang-format off

/** Is a numeric signed integer.
 *
 * The following types are numeric integers: signed char,
 * signed short, signed int, signed long, signed long long.
 *
 * @tparam T type to check for being a numeric integer.
 */
template<typename T> struct is_numeric_signed_integral : std::false_type {}; 
template<> struct is_numeric_signed_integral<signed char> : std::true_type {};
template<> struct is_numeric_signed_integral<signed short> : std::true_type {};
template<> struct is_numeric_signed_integral<signed int> : std::true_type {};
template<> struct is_numeric_signed_integral<signed long> : std::true_type {};
template<> struct is_numeric_signed_integral<signed long long> : std::true_type {};

/** @sa is_numeric_signed_integral
 */
template<typename T>
inline constexpr bool is_numeric_signed_integral_v = is_numeric_signed_integral<T>::value;

/** Is a numeric unsigned integer.
 *
 * The following types are numeric integers: unsigned char,
 * unsigned short, unsigned int, unsigned long, unsigned long long
 *
 * @tparam T type to check for being a numeric integer.
 */
template<typename T> struct is_numeric_unsigned_integral : std::false_type {}; 
template<> struct is_numeric_unsigned_integral<unsigned int> : std::true_type {};
template<> struct is_numeric_unsigned_integral<unsigned char> : std::true_type {};
template<> struct is_numeric_unsigned_integral<unsigned short> : std::true_type {};
template<> struct is_numeric_unsigned_integral<unsigned long> : std::true_type {};
template<> struct is_numeric_unsigned_integral<unsigned long long> : std::true_type {};

/** @sa is_numeric_unsigned_integral
 */
template<typename T>
inline constexpr bool is_numeric_unsigned_integral_v = is_numeric_unsigned_integral<T>::value;

/** Is a numeric integer.
 *
 * The following types are numeric integers: signed char, unsigned char,
 * signed short, unsigned short, signed int, unsigned int, signed long,
 * unsigned long, signed long long, unsigned long long
 *
 * @tparam T type to check for being a numeric integer.
 */
template<typename T> struct is_numeric_integral : std::false_type {}; 
template<> struct is_numeric_integral<unsigned int> : std::true_type {};
template<> struct is_numeric_integral<unsigned char> : std::true_type {};
template<> struct is_numeric_integral<unsigned short> : std::true_type {};
template<> struct is_numeric_integral<unsigned long> : std::true_type {};
template<> struct is_numeric_integral<unsigned long long> : std::true_type {};
template<> struct is_numeric_integral<signed char> : std::true_type {};
template<> struct is_numeric_integral<signed short> : std::true_type {};
template<> struct is_numeric_integral<signed int> : std::true_type {};
template<> struct is_numeric_integral<signed long> : std::true_type {};
template<> struct is_numeric_integral<signed long long> : std::true_type {};

/** @sa is_numeric_integral
 */
template<typename T> inline constexpr bool is_numeric_integral_v = is_numeric_integral<T>::value;

/** Is a numeric.
 *
 * The following types are numeric: signed char, unsigned char,
 * signed short, unsigned short, signed int, unsigned int, signed long,
 * unsigned long, signed long long, unsigned long long, register_long,
 * half, float, double, long double
 *
 * @tparam T type to check for being a numeric integer.
 */
template<typename T> struct is_numeric : std::false_type {}; 
template<> struct is_numeric<unsigned char> : std::true_type {};
template<> struct is_numeric<unsigned short> : std::true_type {};
template<> struct is_numeric<unsigned int> : std::true_type {};
template<> struct is_numeric<unsigned long> : std::true_type {};
template<> struct is_numeric<unsigned long long> : std::true_type {};
template<> struct is_numeric<signed char> : std::true_type {};
template<> struct is_numeric<signed short> : std::true_type {};
template<> struct is_numeric<signed int> : std::true_type {};
template<> struct is_numeric<signed long> : std::true_type {};
template<> struct is_numeric<signed long long> : std::true_type {};
template<> struct is_numeric<float> : std::true_type {};
template<> struct is_numeric<double> : std::true_type {};
template<> struct is_numeric<long double> : std::true_type {};

/** @sa is_numeric
 */
template<typename T> inline constexpr bool is_numeric_v = is_numeric<T>::value;

template<typename T> struct is_character : std::false_type {}; 
template<> struct is_character<char> : std::true_type {};
template<> struct is_character<wchar_t> : std::true_type {};
template<> struct is_character<char8_t> : std::true_type {};
template<> struct is_character<char16_t> : std::true_type {};
template<> struct is_character<char32_t> : std::true_type {};

/*! True is the supplied type is a character integer.
* This distinguishes between integer characters and integer numbers.
*/
template<typename T> inline constexpr bool is_character_v = is_character<T>::value;

/** type-trait to convert a character to a string type.
 */
template<typename T> struct make_string { };
template<> struct make_string<char> { using type = std::string; };
template<> struct make_string<wchar_t> { using type = std::wstring; };
template<> struct make_string<char8_t> { using type = std::u8string; };
template<> struct make_string<char16_t> { using type = std::u16string; };
template<> struct make_string<char32_t> { using type = std::u32string; };

/** type-trait to convert a character to a string type.
 */
template<typename T> using make_string_t = typename make_string<T>::type;

/** type-trait to convert a character to a string_view type.
 */
template<typename T> struct make_string_view { using type = void; };
template<> struct make_string_view<char> { using type = std::string_view; };
template<> struct make_string_view<wchar_t> { using type = std::wstring_view; };
template<> struct make_string_view<char8_t> { using type = std::u8string_view; };
template<> struct make_string_view<char16_t> { using type = std::u16string_view; };
template<> struct make_string_view<char32_t> { using type = std::u32string_view; };

/** type-trait to convert a character to a string_view type.
 */
template<typename T>
using make_string_view_t = typename make_string_view<T>::type;

template<typename T, typename U>
struct make_promote {
    using type = decltype(static_cast<T>(0) + static_cast<U>(0));
};

template<typename T, typename U>
using make_promote_t = typename make_promote<T,U>::type;

template<typename T, typename Ei=void>
struct make_intmax {
    using type = uintmax_t;
};

template<std::unsigned_integral T>
struct make_intmax<T> {
    using type = uintmax_t;
};

template<std::signed_integral T>
struct make_intmax<T> {
    using type = intmax_t;
};

template<typename T>
using make_intmax_t = typename make_intmax<T>::type;

/** Has an signed integer of a specific size.
 * @tparam N The number of bits of the signed integer.
 */
template<std::size_t N> struct has_intxx : public std::false_type {};

/** Has an unsigned integer of a specific size.
 * @tparam N The number of bits of the unsigned integer.
 */
template<std::size_t N> struct has_uintxx : public std::false_type {};

/** Has an float of a specific size.
 * @tparam N The number of bits of the float.
 */
template<std::size_t N> struct has_floatxx : public std::false_type {};

/** Make an signed integer.
 * @tparam N The number of bits of the signed integer.
 */
template<std::size_t N> struct make_intxx {};

/** Make an unsigned integer.
 * @tparam N The number of bits of the unsigned integer.
 */
template<std::size_t N> struct make_uintxx {};

/** Make an floating point.
 * @tparam N The number of bits of the floating point.
 */
template<std::size_t N> struct make_floatxx {};

#if (TT_COMPILER == TT_CC_CLANG || TT_COMPILER == TT_CC_GCC) && (TT_PROCESSOR == TT_CPU_X64)
template<> struct has_intxx<128> : public std::true_type {};
template<> struct has_uintxx<128> : public std::true_type {};
template<> struct make_intxx<128> { using type = __int128; };
template<> struct make_uintxx<128> { using type = unsigned __int128; };
#endif
template<> struct has_intxx<64> : public std::true_type {};
template<> struct has_uintxx<64> : public std::true_type {};
template<> struct has_floatxx<64> : public std::true_type {};
template<> struct make_intxx<64> { using type = int64_t; };
template<> struct make_uintxx<64> { using type = uint64_t; };
template<> struct make_floatxx<64> { using type = double; };
template<> struct has_intxx<32> : public std::true_type {};
template<> struct has_uintxx<32> : public std::true_type {};
template<> struct has_floatxx<32> : public std::true_type {};
template<> struct make_intxx<32> { using type = int32_t; };
template<> struct make_uintxx<32> { using type = uint32_t; };
template<> struct make_floatxx<32> { using type = float; };
template<> struct has_intxx<16> : public std::true_type {};
template<> struct has_uintxx<16> : public std::true_type {};
template<> struct make_intxx<16> { using type = int16_t; };
template<> struct make_uintxx<16> { using type = uint16_t; };
template<> struct has_intxx<8> : public std::true_type {};
template<> struct has_uintxx<8> : public std::true_type {};
template<> struct make_intxx<8> { using type = int8_t; };
template<> struct make_uintxx<8> { using type = uint8_t; };

template<std::size_t N> constexpr bool has_intxx_v = has_intxx<N>::value;
template<std::size_t N> constexpr bool has_uintxx_v = has_uintxx<N>::value;
template<std::size_t N> constexpr bool has_floatxx_v = has_floatxx<N>::value;
template<std::size_t N> using make_intxx_t = typename make_intxx<N>::type;
template<std::size_t N> using make_uintxx_t = typename make_uintxx<N>::type;
template<std::size_t N> using make_floatxx_t = typename make_floatxx<N>::type;


/** Type-trait to copy const volitile qualifiers from one type to another.
 */
template<typename To, typename From, typename Ei=void>
struct copy_cv {};

template<typename To, typename From> requires(not std::is_const_v<From> and not std::is_volatile_v<From>)
struct copy_cv<To,From> {
    using type = std::remove_cv_t<To>;
};

template<typename To, typename From> requires(not std::is_const_v<From> and std::is_volatile_v<From>)
struct copy_cv<To,From> {
    using type = std::remove_cv_t<To> volatile;
};

template<typename To, typename From> requires(std::is_const_v<From> and not std::is_volatile_v<From>)
struct copy_cv<To,From> {
    using type = std::remove_cv_t<To> const;
};

template<typename To, typename From> requires(std::is_const_v<From> and std::is_volatile_v<From>)
struct copy_cv<To,From> {
    using type = std::remove_cv_t<To> const volatile;
};

/** Type-trait to copy const volatile qualifiers from one type to another.
 */
template<typename To, typename From>
using copy_cv_t = typename copy_cv<To,From>::type;

template <typename T> struct has_value_type 
{
    template <typename C> static std::true_type test(typename C::value_type *);
    template <typename> static std::false_type test(...);

    static const bool value = std::is_same_v<decltype(test<T>(nullptr)), std::true_type>;
};

template<typename T>
inline constexpr bool has_value_type_v = has_value_type<T>::value;

template <typename T> struct has_add_callback 
{
    template <typename C> static std::true_type test(decltype(&C::add_callback) func_ptr);
    template <typename> static std::false_type test(...);

    static const bool value = std::is_same_v<decltype(test<T>(nullptr)), std::true_type>;
};

template<typename T>
inline constexpr bool has_add_callback_v = has_add_callback<T>::value;

template<typename BaseType, typename DerivedType>
struct is_decayed_base_of : public std::is_base_of<std::decay_t<BaseType>,std::decay_t<DerivedType>> {};

template<typename BaseType, typename DerivedType>
constexpr bool is_decayed_base_of_v = is_decayed_base_of<BaseType,DerivedType>::value;

template<typename DerivedType, typename BaseType>
struct is_derived_from : public std::is_base_of<BaseType,DerivedType> {};

template<typename DerivedType, typename BaseType>
constexpr bool is_derived_from_v = is_derived_from<DerivedType,BaseType>::value;

template<typename DerivedType, typename BaseType>
struct is_decayed_derived_from : public is_decayed_base_of<BaseType,DerivedType> {};

template<typename DerivedType, typename BaseType>
constexpr bool is_decayed_derived_from_v = is_decayed_derived_from<DerivedType,BaseType>::value;

template<typename T1, typename T2>
constexpr bool is_different_v = !std::is_same_v<T1,T2>;

template<typename T>
struct is_atomic : public std::false_type {};

template<typename T>
struct is_atomic<std::atomic<T>> : public std::true_type {};

template<typename T>
constexpr bool is_atomic_v = is_atomic<T>::value;

template<typename First, typename Second>
struct use_first {
    using type = First;
};

template<typename First, typename Second>
using use_first_t = use_first<First,Second>;

template<typename T>
struct acts_as_pointer : public std::false_type {};

template<typename T> struct acts_as_pointer<std::shared_ptr<T>> : public std::true_type {};
template<typename T> struct acts_as_pointer<std::shared_ptr<T> &&> : public std::true_type {};
template<typename T> struct acts_as_pointer<std::shared_ptr<T> &> : public std::true_type {};
template<typename T> struct acts_as_pointer<std::shared_ptr<T> const &> : public std::true_type {};
template<typename T> struct acts_as_pointer<std::weak_ptr<T>> : public std::true_type {};
template<typename T> struct acts_as_pointer<std::weak_ptr<T> &&> : public std::true_type {};
template<typename T> struct acts_as_pointer<std::weak_ptr<T> &> : public std::true_type {};
template<typename T> struct acts_as_pointer<std::weak_ptr<T> const &> : public std::true_type {};
template<typename T> struct acts_as_pointer<std::unique_ptr<T>> : public std::true_type {};
template<typename T> struct acts_as_pointer<std::unique_ptr<T> &&> : public std::true_type {};
template<typename T> struct acts_as_pointer<std::unique_ptr<T> &> : public std::true_type {};
template<typename T> struct acts_as_pointer<std::unique_ptr<T> const &> : public std::true_type {};
template<typename T> struct acts_as_pointer<T *> : public std::true_type {};

template<typename T>
constexpr bool acts_as_pointer_v = acts_as_pointer<T>::value;

#define tt_call_method(object, method, ...) \
    [&]() { \
        if constexpr (acts_as_pointer_v<decltype(object)>) { \
            return object->method(__VA_ARGS__); \
        } else { \
            return object.method(__VA_ARGS__); \
        } \
    }()

// clang-format on

/** All values of numeric type `In` can be represented without loss of precision by numeric type `Out`.
 */
template<typename Out, typename In>
constexpr bool type_in_range_v = std::numeric_limits<Out>::digits >= std::numeric_limits<In>::digits and
    (std::numeric_limits<Out>::is_signed == std::numeric_limits<In>::is_signed or std::numeric_limits<Out>::is_signed);

/** True if T is a forwarded type of OfType.
* 
* ```
* template<forward_of<std::string> Text>
* std::string foo(Text &&text) {
*   return std::forward<Text>(text);
* }
* ```
 */
template<typename T, typename Forward>
struct is_forward_of : public std::false_type {};

template<typename T>
struct is_forward_of<T, T> : public std::true_type {};

template<typename T>
struct is_forward_of<T, T const &> : public std::true_type {
};

template<typename T>
struct is_forward_of<T, T &> : public std::true_type {
};

template<typename T, typename Forward>
constexpr bool is_forward_of_v = is_forward_of<T, Forward>::value;

} // namespace tt::inline v1
