
#pragma once

#include <cstdint>
#include <type_traits>

namespace tt {

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
template<> struct is_character<char16_t> : std::true_type {};
template<> struct is_character<char32_t> : std::true_type {};

/*! True is the supplied type is a character integer.
* This distingueses between integer characters and integer numbers.
*/
template<typename T>
inline constexpr bool is_character_v = is_character<T>::value;


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

template<typename T>
using make_larger_t = typename make_larger<T>::type;

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

template<typename To, typename From>
using copy_cv_t = typename copy_cv<To,From>::type;

//template<typename T>
//struct remove_cvref {
//    using type = std::remove_cv_t<std::remove_reference_t<T>>;
//};
//
//template<typename T>
//using remove_cvref_t = typename remove_cvref<T>::type;

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

template<typename T, typename Enable=void>
struct make_value_type {};

template<typename T>
struct make_value_type<T, std::enable_if_t<!has_value_type_v<T>>> { using type = T; };

template<typename T>
struct make_value_type<T, std::enable_if_t<has_value_type_v<T>>> { using type = typename T::value_type; };

template<typename T>
using make_value_type_t = typename make_value_type<T>::type;

template<typename DerivedType, typename BaseType>
struct is_derived_from : public std::is_base_of<BaseType,DerivedType> {};

template<typename DerivedType, typename BaseType>
constexpr bool is_derived_from_v = is_derived_from<DerivedType,BaseType>::value;

}
