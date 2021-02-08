
#pragma once

#include <cstdint>
#include <type_traits>
#include <string>
#include <string_view>

namespace tt {

// clang-format off
template<typename T> struct is_numeric_integer : std::false_type {}; 
template<> struct is_numeric_integer<signed char> : std::true_type {};
template<> struct is_numeric_integer<unsigned char> : std::true_type {};
template<> struct is_numeric_integer<signed short> : std::true_type {};
template<> struct is_numeric_integer<unsigned short> : std::true_type {};
template<> struct is_numeric_integer<signed int> : std::true_type {};
template<> struct is_numeric_integer<unsigned int> : std::true_type {};
template<> struct is_numeric_integer<signed long> : std::true_type {};
template<> struct is_numeric_integer<unsigned long> : std::true_type {};
template<> struct is_numeric_integer<signed long long> : std::true_type {};
template<> struct is_numeric_integer<unsigned long long> : std::true_type {};

/*! True is the supplied type is a numeric integer.
 * This distinguishes between integer characters/bytes/boolean and integer numbers.
 */
template<typename T>
inline constexpr bool is_numeric_integer_v = is_numeric_integer<T>::value;

template<typename T> struct is_character : std::false_type {}; 
template<> struct is_character<char> : std::true_type {};
template<> struct is_character<wchar_t> : std::true_type {};
template<> struct is_character<char8_t> : std::true_type {};
template<> struct is_character<char16_t> : std::true_type {};
template<> struct is_character<char32_t> : std::true_type {};

/*! True is the supplied type is a character integer.
* This distinguishes between integer characters and integer numbers.
*/
template<typename T>
inline constexpr bool is_character_v = is_character<T>::value;

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
template<typename T>
using make_string_t = typename make_string<T>::type;

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

template<typename T>
struct make_intmax<T,std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>>> {
    using type = uintmax_t;
};

template<typename T>
struct make_intmax<T,std::enable_if_t<std::is_integral_v<T> && std::is_signed_v<T>>> {
    using type = intmax_t;
};

template<typename T>
using make_intmax_t = typename make_intmax<T>::type;

/** Type-trait to increase the size of an integral type.
 */
template<typename T> struct make_larger { using type = T; };
template<> struct make_larger<signed long> { using type = signed long long; };
template<> struct make_larger<signed int> { using type = signed long; };
template<> struct make_larger<signed short> { using type = signed int; };
template<> struct make_larger<signed char> { using type = signed short; };
template<> struct make_larger<unsigned long> { using type = unsigned long long; };
template<> struct make_larger<unsigned int> { using type = unsigned long; };
template<> struct make_larger<unsigned short> { using type = unsigned int; };
template<> struct make_larger<unsigned char> { using type = unsigned short; };
template<> struct make_larger<double> { using type = long double; };
template<> struct make_larger<float> { using type = double; };

/** Type-trait to increase the size of an integral type.
 */
template<typename T>
using make_larger_t = typename make_larger<T>::type;

/** Type-trait to copy const volitile qualifiers from one type to another.
 */
template<typename To, typename From, typename Ei=void>
struct copy_cv {};

template<typename To, typename From>
struct copy_cv<To,From,std::enable_if_t<!std::is_const_v<From> && !std::is_volatile_v<From>>> {
    using type = std::remove_cv_t<To>;
};

template<typename To, typename From>
struct copy_cv<To,From,std::enable_if_t<!std::is_const_v<From> && std::is_volatile_v<From>>> {
    using type = std::remove_cv_t<To> volatile;
};

template<typename To, typename From>
struct copy_cv<To,From,std::enable_if_t<std::is_const_v<From> && !std::is_volatile_v<From>>> {
    using type = std::remove_cv_t<To> const;
};

template<typename To, typename From>
struct copy_cv<To,From,std::enable_if_t<std::is_const_v<From> && std::is_volatile_v<From>>> {
    using type = std::remove_cv_t<To> const volatile;
};

/** Type-trait to copy const volitile qualifiers from one type to another.
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


template<typename First, typename Second>
struct use_first {
    using type = First;
};

template<typename First, typename Second>
using use_first_t = use_first<First,Second>;

}
