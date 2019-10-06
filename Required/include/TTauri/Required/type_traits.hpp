
#pragma once

#include <type_traits>

namespace TTauri {

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
 * This distingueses between integer characters/bytes/boolean and integer numbers.
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
struct promote {
    using type = decltype(static_cast<T>(0) + static_cast<U>(0));
};

template<typename T, typename U>
using promote_t = promote<T,U>::type;

}
