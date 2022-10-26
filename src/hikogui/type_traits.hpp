// Copyright Take Vos 2019-2022.
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
#include <variant>

namespace hi::inline v1 {

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

#if (HI_COMPILER == HI_CC_CLANG || HI_COMPILER == HI_CC_GCC) && (HI_PROCESSOR == HI_CPU_X64)
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

/** Get an integer type that will fit all values from all template parameters.
 *
 * If there is a mix of signed and unsigned integer types, then:
 * 1. all unsigned integers are upgraded to a larger signed integer, then
 * 2. the largest signed integer is returned in `type`.
 *
 * If all integers are unsigned or all integers are signed, then:
 * 1. the largest integer is returned in `type`.
 *
 * @tparam L An integer type.
 * @tparam R Other integer types.
 */
template<typename L, typename... R>
struct common_integer;

template<std::integral T>
struct common_integer<T,T> {
    using type = T;
};

template<std::unsigned_integral L, std::unsigned_integral R>
struct common_integer<L,R> {
    using type = std::conditional_t<(sizeof(L) > sizeof(R)), L, R>;
};

template<std::signed_integral L, std::signed_integral R>
struct common_integer<L,R> {
    using type = std::conditional_t<(sizeof(L) > sizeof(R)), L, R>;
};

template<std::unsigned_integral L, std::signed_integral R>
struct common_integer<L,R> {
    using _left_type =
        std::conditional_t<(sizeof(L) < sizeof(short)), short,
        std::conditional_t<(sizeof(L) < sizeof(int)), int,
        std::conditional_t<(sizeof(L) < sizeof(long)), long, long long>>>;
        
    using type = common_integer<_left_type, R>::type;
};

template<std::signed_integral L, std::unsigned_integral R>
struct common_integer<L,R> {
    using type = common_integer<R, L>::type;
};

template<std::integral L, std::integral M, std::integral... R>
struct common_integer<L, M, R...> {
    using type = common_integer<L, typename common_integer<M, R...>::type>::type;
};

/** Get an integer type that will fit all values from all template parameters.
 *
 * If there is a mix of signed and unsigned integer types, then:
 * 1. all unsigned integers are upgraded to a larger signed integer, then
 * 2. the largest signed integer is returned.
 *
 * If all integers are unsigned or all integers are signed, then:
 * 1. the largest integer is returned.
 *
 * @tparam L An integer type.
 * @tparam R Other integer types.
 */
template<std::integral L, std::integral... R>
using common_integer_t = common_integer<L, R...>::type;


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

/** If the types are different.
* The two types are checked after removing const, volatile and reference qualifiers.
 */
template<typename Context, typename Expected>
constexpr bool is_different_v = not std::is_same_v<std::decay_t<Context>,std::decay_t<Expected>>;

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

/** Smart pointer traits.
 *
 * @note Applications may make specializations for their own types.
 * @param T the type.
 */
template<typename T>
struct smart_pointer_traits {
    /** If true this is a pointer or shared_ptr.
     */
    constexpr static bool value = false;

    /** The type the pointer points to.
     */
    using type = void;
};

template<typename T> struct smart_pointer_traits<std::shared_ptr<T>> {constexpr static bool value = true; using type = T;};
template<typename T> struct smart_pointer_traits<std::weak_ptr<T>> {constexpr static bool value = true; using type = T;};
template<typename T> struct smart_pointer_traits<std::unique_ptr<T>> {constexpr static bool value = true; using type = T;};
template<typename T> struct smart_pointer_traits<T *> {constexpr static bool value = true; using type = T;};

/** Call a method on a reference or a pointer object.
 *
 * @param object A reference or pointer to an object with a method.
 * @param method The name of the method to call.
 * @param ... Argument passed to the method.
 * @return The return value of the method.
 */
#define hi_call_method(object, method, ...) \
    [&]() { \
        if constexpr (smart_pointer_traits<std::decay_t<decltype(object)>>::value) { \
            return object->method(__VA_ARGS__); \
        } else { \
            return object.method(__VA_ARGS__); \
        } \
    }()

/** All values of numeric type `In` can be represented without loss of precision by numeric type `Out`.
 */
template<typename Out, typename In>
constexpr bool type_in_range_v = std::numeric_limits<Out>::digits >= std::numeric_limits<In>::digits and
    (std::numeric_limits<Out>::is_signed == std::numeric_limits<In>::is_signed or std::numeric_limits<Out>::is_signed);

/** Is context a form of the expected type.
 *
 * The context matched the expected type when:
 *  - expected is a non-reference type and the decayed context is the same type, or derived from expected.
 *  - expected is a pointer, shared_ptr, weak_ptr or unique_ptr and the context is convertible to expected.
 *  - expected is in the form of `Result(Arguments...)` and the context is convertible to expected.
 *  - expected is a observer and the context is convertible to expected.
 *  - multiple expected are `or`-ed together
 *
 * Examples of `forward_of` concept which is created from `is_forward_of`:
 *
 * ```
 * void foo(forward_of<std::string> auto &&text) {
 *     bar(std::forward<decltype(text)>(text));
 * }

 * void foo(forward_of<std::shared_ptr<std::string>> auto &&ptr) {
 *     bar(std::forward<decltype(text)>(ptr));
 * }
 *
 * void foo(forward_of<std::unique_ptr<std::string>> auto &&ptr) {
 *     bar(std::forward<decltype(text)>(ptr));
 * }
 *
 * void foo(forward_of<std::string *> auto &&ptr) {
 *     bar(std::forward<decltype(text)>(ptr));
 * }
 *
 * void foo(forward_of<void(std::string)> auto &&func) {
 *     bar(std::forward<decltype(func)>(func));
 * }
 * ```
 *
 * @tparam Context The template argument of a forwarding function argument.
 * @tparam Expected The type expected that matched a decayed-context.
 * @tparam OtherExpected optional other expected types which may match the decayed-context.
 */
template<typename Context, typename Expected, typename... OtherExpected>
struct is_forward_of;

template<typename Context, typename Expected, typename FirstOtherExpected, typename... OtherExpected>
struct is_forward_of<Context, Expected, FirstOtherExpected, OtherExpected...> : std::conditional_t<
        is_forward_of<Context, Expected>::value or
        (is_forward_of<Context, FirstOtherExpected>::value or ... or is_forward_of<Context, OtherExpected>::value),
        std::true_type,
        std::false_type> {};

template<typename Context, typename Expected>
struct is_forward_of<Context, Expected> :
    std::conditional_t<
        std::is_same_v<std::decay_t<Context>, Expected> or std::is_base_of_v<Expected, std::decay_t<Context>>,
        std::true_type,
        std::false_type> {
    static_assert(not std::is_reference_v<Expected>, "Template argument Expected must be a non-reference type.");
};

template<typename Context, typename Expected>
struct is_forward_of<Context, std::shared_ptr<Expected>> :
    std::conditional_t<
        std::is_convertible_v<Context, std::shared_ptr<Expected>>,
        std::true_type,
        std::false_type> {};

template<typename Context, typename Expected>
struct is_forward_of<Context, std::weak_ptr<Expected>> :
    std::conditional_t<
        std::is_convertible_v<Context, std::weak_ptr<Expected>>,
        std::true_type,
        std::false_type> {};

template<typename Context, typename Expected>
struct is_forward_of<Context, std::unique_ptr<Expected>> :
    std::conditional_t<
        std::is_convertible_v<Context, std::unique_ptr<Expected>>,
        std::true_type,
        std::false_type> {};

template<typename Context, typename Expected>
struct is_forward_of<Context, Expected *> :
    std::conditional_t<
        std::is_convertible_v<Context, Expected *>,
        std::true_type,
        std::false_type> {};

template<typename Context, typename Result, typename... Args>
struct is_forward_of<Context, Result(Args...)> :
    std::conditional_t<std::is_invocable_r_v<Result, Context, Args...>, std::true_type, std::false_type> {
};

template<typename Context, typename Expected, typename... OtherExpected>
constexpr bool is_forward_of_v = is_forward_of<Context, Expected, OtherExpected...>::value;

template<typename Context>
struct forward_copy_or_ref {
    using type = std::conditional_t<std::is_rvalue_reference_v<Context>, std::decay_t<Context>, std::decay_t<Context> const &>;
};

template<typename Context>
using forward_copy_or_ref_t = forward_copy_or_ref<Context>::type;


/** Decays types for use as elements in std::variant.
 *
 * @tparam T type to be decayed, or when `void` converted to `std::monostate`.
 */
template<typename T>
struct variant_decay {
    using type = std::decay_t<T>;
};

template<>
struct variant_decay<void> {
    using type = std::monostate;
};

/** @see variant_decay
 */
template<typename T>
using variant_decay_t = variant_decay<T>::type;

/** This selector allows access to member variable by name.
 *
 * An application may add a specialization for `selector` for its own type.
 * The specialization should add a templated function `get()` to give access to members
 * based on the template parameter.
 *
 * The prototype of the `get()` function are as follows:
 *  - `template<fixed_string> auto &get(T &) const noexcept`
 *
 * Here is an example how to specialize `hi::selector` for the `my::simple` type:
 *
 * ```cpp
 * namespace my {
 * struct simple {
 *     int foo;
 *     std::string bar;
 * };
 * }
 *
 * template<>
 * struct hi::selector<my::simple> {
 *     template<hi::fixed_string> auto &get(my::simple &) const noexcept;
 *
 *     template<> auto &get<"foo">(my::simple &rhs) const noexcept { return rhs.foo; }
 *     template<> auto &get<"bar">(my::simple &rhs) const noexcept { return rhs.bar; }
 * };
 * ```
 */
template<typename T>
struct selector;


/** Documentation of a type.
 *
 *
 *
 *
 *
 *
 * ```cpp
 * namespace my {
 * struct simple {
 *     int foo;
 *     std::string bar;
 * };
 * }
 *
 * template<>
 * struct hi::type_documentation<my::simple> {
 *     std::vector<std::string_view> names();
 * };
 * ```
 */
template<typename T>
struct type_documentation;



/** Helper type to turn a set of lambdas into a single overloaded type to pass to `std::visit()`.
* 
* @tparam Ts A set of lambdas with a single argument.
*/
template<class... Ts>
struct overloaded : Ts... {
    // Makes the call operator of each lambda available directly to the `overloaded` type. 
    using Ts::operator()...;

    // The implicit constructor is all that is needed to initialize each lambda. 
};

} // namespace hi::inline v1
