// Copyright 2024 Aurora Operations, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <limits>
#include <ostream>
#include <ratio>
#include <type_traits>
#include <utility>

// Version identifier: 0.3.4-5-gffea274
// <iostream> support: INCLUDED
// List of included units:
//   amperes
//   bits
//   candelas
//   grams
//   kelvins
//   meters
//   moles
//   radians
//   seconds
//   unos


namespace au {

// A type representing a quantity of "zero" in any units.
//
// Zero is special: it's the only number that we can meaningfully compare or assign to a Quantity of
// _any_ dimension.  Giving it a special type (and a predefined constant of that type, `ZERO`,
// defined below) lets our code be both concise and readable.
//
// For example, we can zero-initialize any arbitrary Quantity, even if it doesn't have a
// user-defined literal, and even if it's in a header file so we couldn't use the literals anyway:
//
//   struct PathPoint {
//       QuantityD<RadiansPerMeter> curvature = ZERO;
//   };
struct Zero {
    // Implicit conversion to arithmetic types.
    template <typename T, typename Enable = std::enable_if_t<std::is_arithmetic<T>::value>>
    constexpr operator T() const {
        return 0;
    }

    // Implicit conversion to chrono durations.
    template <typename Rep, typename Period>
    constexpr operator std::chrono::duration<Rep, Period>() const {
        return std::chrono::duration<Rep, Period>{0};
    }
};

// A value of Zero.
//
// This exists purely for convenience, so people don't have to call the initializer.  i.e., it lets
// us write `ZERO` instead of `Zero{}`.
static constexpr auto ZERO = Zero{};

// Addition, subtraction, and comparison of Zero are well defined.
inline constexpr Zero operator+(Zero, Zero) { return ZERO; }
inline constexpr Zero operator-(Zero, Zero) { return ZERO; }
inline constexpr bool operator==(Zero, Zero) { return true; }
inline constexpr bool operator>=(Zero, Zero) { return true; }
inline constexpr bool operator<=(Zero, Zero) { return true; }
inline constexpr bool operator!=(Zero, Zero) { return false; }
inline constexpr bool operator>(Zero, Zero) { return false; }
inline constexpr bool operator<(Zero, Zero) { return false; }

}  // namespace au


namespace au {
namespace stdx {

// Source: adapted from (https://en.cppreference.com/w/cpp/types/type_identity).
template <class T>
struct type_identity {
    using type = T;
};

// Source: adapted from (https://en.cppreference.com/w/cpp/types/integral_constant).
template <bool B>
using bool_constant = std::integral_constant<bool, B>;

// Source: adapted from (https://en.cppreference.com/w/cpp/types/conjunction).
template <class...>
struct conjunction : std::true_type {};
template <class B1>
struct conjunction<B1> : B1 {};
template <class B1, class... Bn>
struct conjunction<B1, Bn...> : std::conditional_t<bool(B1::value), conjunction<Bn...>, B1> {};

// Source: adapted from (https://en.cppreference.com/w/cpp/types/disjunction).
template <class...>
struct disjunction : std::false_type {};
template <class B1>
struct disjunction<B1> : B1 {};
template <class B1, class... Bn>
struct disjunction<B1, Bn...> : std::conditional_t<bool(B1::value), B1, disjunction<Bn...>> {};

// Source: adapted from (https://en.cppreference.com/w/cpp/types/negation).
template <class B>
struct negation : stdx::bool_constant<!static_cast<bool>(B::value)> {};

// Source: adapted from (https://en.cppreference.com/w/cpp/types/remove_cvref).
template <class T>
struct remove_cvref {
    typedef std::remove_cv_t<std::remove_reference_t<T>> type;
};
template <class T>
using remove_cvref_t = typename remove_cvref<T>::type;

// Source: adapted from (https://en.cppreference.com/w/cpp/types/void_t).
template <class...>
using void_t = void;

}  // namespace stdx
}  // namespace au


namespace au {
namespace detail {

//
// A constexpr-compatible string constant class of a given size.
//
// The point is to make it easy to build up compile-time strings by using "join" and "concatenate"
// operations.
//
// Beware that this is not one type, but a family of types, one for each length!  If you're in a
// context where you can't use `auto` (say, because you're making a member variable), you'll need to
// know the length in order to name the type.
//
template <std::size_t Strlen>
class StringConstant;

//
// `as_string_constant()`: Create StringConstant<N>, of correct length, corresponding to the input.
//
// Possible inputs include char-arrays, or `StringConstant` (for which this is the identity).
//
template <std::size_t N>
constexpr StringConstant<N - 1> as_string_constant(const char (&c_string)[N]) {
    return {c_string};
}
template <std::size_t N>
constexpr StringConstant<N> as_string_constant(const StringConstant<N> &x) {
    return x;
}

//
// Create StringConstant which concatenates all arguments.
//
// Each argument will be treated as a StringConstant.  The final length will automatically be
// computed from the lengths of the inputs.
//
template <typename... Ts>
constexpr auto concatenate(const Ts &...ts);

//
// Join arbitrarily many arguments into a new StringConstant, using the first argument as separator.
//
// Each argument will be treated as a StringConstant.  The final length will automatically be
// computed from the lengths of the inputs.
//
// As usual for the join algorithm, the separator will not appear in the output unless there are at
// least two arguments (apart from the separator) being joined.
//
template <typename SepT, typename... StringTs>
constexpr auto join_by(const SepT &sep, const StringTs &...ts);

//
// Wrap a StringConstant in parentheses () if the supplied template param is true.
//
template <bool Enable, typename StringT>
constexpr auto parens_if(const StringT &s);

//
// A constexpr-compatible utility to generate compile-time string representations of integers.
//
// `IToA<N>::value` is a `StringConstant<Len>` of whatever appropriate length `Len` is needed to
// represent `N`.  This includes handling the negative sign (if any).
//
template <int64_t N>
struct IToA;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation details below.
////////////////////////////////////////////////////////////////////////////////////////////////////

// The string-length needed to hold a representation of this integer.
constexpr std::size_t string_size(int64_t x) {
    if (x < 0) {
        return string_size(-x) + 1;
    }

    std::size_t digits = 1;
    while (x > 9) {
        x /= 10;
        ++digits;
    }
    return digits;
}

// The sum of the template parameters.
template <std::size_t... Ns>
constexpr std::size_t sum() {
    std::size_t result{0};
    std::size_t values[] = {0u, Ns...};  // Dummy `0u` avoids empty array.
    for (std::size_t i = 0; i < sizeof...(Ns); ++i) {
        result += values[i + 1u];  // "+ 1u" to skip the dummy value.
    }
    return result;
}

template <std::size_t Strlen>
class StringConstant {
 public:
    constexpr StringConstant(const char (&c_string)[Strlen + 1])
        : StringConstant{c_string, std::make_index_sequence<Strlen>{}} {}

    static constexpr std::size_t length = Strlen;

    // Get a C-string representation of this constant.
    //
    // (Note that the constructors have guaranteed a correct placement of the '\0'.)
    constexpr const char *c_str() const { return data_array_; }
    constexpr operator const char *() const { return c_str(); }

    // Get a (sizeof()-compatible) char array reference to the data.
    constexpr auto char_array() const -> const char (&)[Strlen + 1] { return data_array_; }

    // The string-length of this string (i.e., NOT including the null terminator).
    constexpr std::size_t size() const { return Strlen; }

    // Whether this string is empty (i.e., has size zero).
    constexpr bool empty() const { return size() == 0u; }

    // Join multiple `StringConstant<Ns>` inputs, using `*this` as the separator.
    template <std::size_t... Ns>
    constexpr auto join(const StringConstant<Ns> &...items) const {
        constexpr std::size_t N =
            sum<Ns...>() + Strlen * (sizeof...(items) > 0 ? (sizeof...(items) - 1) : 0);
        char result[N + 1]{'\0'};
        join_impl(result, items...);
        return StringConstant<N>{result};
    }

 private:
    // This would be unsafe if called with arbitrary pointers and/or integer sequences.  However,
    // this is a private constructor of this class, called only by its public constructor(s), and we
    // know they satisfy the conditions needed to call this function safely.
    template <std::size_t... Is>
    constexpr StringConstant(const char *data, std::index_sequence<Is...>)
        : data_array_{data[Is]..., '\0'} {
        (void)data;  // Suppress unused-var error when `Is` is empty in platform-independent way.
    }

    // Base case for the join algorithm.
    constexpr void join_impl(char *) const {}

    // Recursive case for the join algorithm.
    template <std::size_t N, std::size_t... Ns>
    constexpr void join_impl(char *out_iter,
                             const StringConstant<N> &head,
                             const StringConstant<Ns> &...tail) const {
        // Definitely copy data from the head element.
        //
        // The `static_cast<int>` incantation mollifies certain "helpful" compilers, which notice
        // that the comparison is always false when `Strlen` is `0`, and disregard best practices
        // for generic programming by failing the build for this.
        for (std::size_t i = 0; static_cast<int>(i) < static_cast<int>(N); ++i) {
            *out_iter++ = head.c_str()[i];
        }

        // If there are tail elements, copy out the separator, and recurse.
        if (sizeof...(tail) > 0) {
            // The `static_cast<int>` incantation mollifies certain "helpful" compilers, which
            // notice that the comparison is always false when `Strlen` is `0`, and disregard best
            // practices for generic programming by failing the build for this.
            for (std::size_t i = 0; static_cast<int>(i) < static_cast<int>(Strlen); ++i) {
                *out_iter++ = data_array_[i];
            }
            join_impl(out_iter, tail...);
        }
    }

    // Data storage for the string constant.
    const char data_array_[Strlen + 1];
};

template <std::size_t Strlen>
constexpr std::size_t StringConstant<Strlen>::length;

template <typename... Ts>
constexpr auto concatenate(const Ts &...ts) {
    return join_by("", ts...);
}

template <typename SepT, typename... StringTs>
constexpr auto join_by(const SepT &sep, const StringTs &...ts) {
    return as_string_constant(sep).join(as_string_constant(ts)...);
}

template <int64_t N>
struct IToA {
 private:
    static constexpr auto print_to_array() {
        char data[length + 1] = {'\0'};

        int num = N;
        if (num < 0) {
            data[0] = '-';
            num = -num;
        }

        std::size_t i = length - 1;
        do {
            data[i--] = '0' + static_cast<char>(num % 10);
            num /= 10;
        } while (num > 0);

        return StringConstant<length>{data};
    }

 public:
    static constexpr std::size_t length = string_size(N);

    static constexpr StringConstant<length> value = print_to_array();
};

// Definitions for IToA<N>::value.  (Needed to prevent linker errors.)
template <int64_t N>
constexpr std::size_t IToA<N>::length;
template <int64_t N>
constexpr StringConstant<IToA<N>::length> IToA<N>::value;

template <bool Enable>
struct ParensIf;

template <>
struct ParensIf<true> {
    static constexpr StringConstant<1> open() { return as_string_constant("("); }
    static constexpr StringConstant<1> close() { return as_string_constant(")"); }
};

template <>
struct ParensIf<false> {
    static constexpr StringConstant<0> open() { return as_string_constant(""); }
    static constexpr StringConstant<0> close() { return as_string_constant(""); }
};

template <bool Enable, typename StringT>
constexpr auto parens_if(const StringT &s) {
    return concatenate(ParensIf<Enable>::open(), s, ParensIf<Enable>::close());
}

}  // namespace detail
}  // namespace au


namespace au {

////////////////////////////////////////////////////////////////////////////////////////////////////
// Generic mathematical convenience functions.
//
// The reason these exist is to be able to make unit expressions easier to read in common cases.
// They also work for dimensions and magnitudes.

//
// This section works around an error:
//
//    warning: use of function template name with no prior declaration in function call with
//    explicit template arguments is a C++20 extension [-Wc++20-extensions]
//
// We work around it by providing declarations, even though those declarations are never used.
//
namespace no_prior_declaration_workaround {
struct Dummy;
}  // namespace no_prior_declaration_workaround
template <std::intmax_t N>
auto root(no_prior_declaration_workaround::Dummy);
template <std::intmax_t N>
auto pow(no_prior_declaration_workaround::Dummy);

// Make "inverse" an alias for "pow<-1>" when the latter exists (for anything).
template <typename T>
constexpr auto inverse(T x) -> decltype(pow<-1>(x)) {
    return pow<-1>(x);
}

// Make "squared" an alias for "pow<2>" when the latter exists (for anything).
template <typename T>
constexpr auto squared(T x) -> decltype(pow<2>(x)) {
    return pow<2>(x);
}

// Make "cubed" an alias for "pow<3>" when the latter exists (for anything).
template <typename T>
constexpr auto cubed(T x) -> decltype(pow<3>(x)) {
    return pow<3>(x);
}

// Make "sqrt" an alias for "root<2>" when the latter exists (for anything).
template <typename T>
constexpr auto sqrt(T x) -> decltype(root<2>(x)) {
    return root<2>(x);
}

// Make "cbrt" an alias for "root<3>" when the latter exists (for anything).
template <typename T>
constexpr auto cbrt(T x) -> decltype(root<3>(x)) {
    return root<3>(x);
}

}  // namespace au


namespace au {
namespace detail {

// Find the smallest factor which divides n.
//
// Undefined unless (n > 1).
constexpr std::uintmax_t find_first_factor(std::uintmax_t n) {
    if (n % 2u == 0u) {
        return 2u;
    }

    std::uintmax_t factor = 3u;
    while (factor * factor <= n) {
        if (n % factor == 0u) {
            return factor;
        }
        factor += 2u;
    }

    return n;
}

// Check whether a number is prime.
constexpr bool is_prime(std::uintmax_t n) { return (n > 1) && (find_first_factor(n) == n); }

// Find the largest power of `factor` which divides `n`.
//
// Undefined unless n > 0, and factor > 1.
constexpr std::uintmax_t multiplicity(std::uintmax_t factor, std::uintmax_t n) {
    std::uintmax_t m = 0u;
    while (n % factor == 0u) {
        ++m;
        n /= factor;
    }
    return m;
}

template <typename T>
constexpr T square(T n) {
    return n * n;
}

// Raise a base to an integer power.
//
// Undefined behavior if base^exp overflows T.
template <typename T>
constexpr T int_pow(T base, std::uintmax_t exp) {
    if (exp == 0u) {
        return T{1};
    }

    if (exp % 2u == 1u) {
        return base * int_pow(base, exp - 1u);
    }

    return square(int_pow(base, exp / 2u));
}

}  // namespace detail
}  // namespace au


namespace au {
namespace stdx {

// Source: adapted from (https://en.cppreference.com/w/cpp/utility/intcmp).
//
// For C++14 compatibility, we needed to change `if constexpr` to SFINAE.
template <typename T, typename U, typename Enable = void>
struct CmpEqualImpl;
template <class T, class U>
constexpr bool cmp_equal(T t, U u) noexcept {
    return CmpEqualImpl<T, U>{}(t, u);
}

// Source: adapted from (https://en.cppreference.com/w/cpp/utility/intcmp).
template <class T, class U>
constexpr bool cmp_not_equal(T t, U u) noexcept {
    return !cmp_equal(t, u);
}

// Source: adapted from (https://en.cppreference.com/w/cpp/utility/intcmp).
//
// For C++14 compatibility, we needed to change `if constexpr` to SFINAE.
template <typename T, typename U, typename Enable = void>
struct CmpLessImpl;
template <class T, class U>
constexpr bool cmp_less(T t, U u) noexcept {
    return CmpLessImpl<T, U>{}(t, u);
}

// Source: adapted from (https://en.cppreference.com/w/cpp/utility/intcmp).
template <class T, class U>
constexpr bool cmp_greater(T t, U u) noexcept {
    return cmp_less(u, t);
}

// Source: adapted from (https://en.cppreference.com/w/cpp/utility/intcmp).
template <class T, class U>
constexpr bool cmp_less_equal(T t, U u) noexcept {
    return !cmp_greater(t, u);
}

// Source: adapted from (https://en.cppreference.com/w/cpp/utility/intcmp).
template <class T, class U>
constexpr bool cmp_greater_equal(T t, U u) noexcept {
    return !cmp_less(t, u);
}

// Source: adapted from (https://en.cppreference.com/w/cpp/utility/in_range).
template <class R, class T>
constexpr bool in_range(T t) noexcept {
    return cmp_greater_equal(t, std::numeric_limits<R>::min()) &&
           cmp_less_equal(t, std::numeric_limits<R>::max());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation details below.
////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T, typename U>
struct CmpEqualImpl<T, U, std::enable_if_t<std::is_signed<T>::value == std::is_signed<U>::value>> {
    constexpr bool operator()(T t, U u) { return t == u; }
};

template <typename T, typename U>
struct CmpEqualImpl<T, U, std::enable_if_t<std::is_signed<T>::value && !std::is_signed<U>::value>> {
    constexpr bool operator()(T t, U u) { return t < 0 ? false : std::make_unsigned_t<T>(t) == u; }
};

template <typename T, typename U>
struct CmpEqualImpl<T, U, std::enable_if_t<!std::is_signed<T>::value && std::is_signed<U>::value>> {
    constexpr bool operator()(T t, U u) { return u < 0 ? false : t == std::make_unsigned_t<U>(u); }
};

template <typename T, typename U>
struct CmpLessImpl<T, U, std::enable_if_t<std::is_signed<T>::value == std::is_signed<U>::value>> {
    constexpr bool operator()(T t, U u) { return t < u; }
};

template <typename T, typename U>
struct CmpLessImpl<T, U, std::enable_if_t<std::is_signed<T>::value && !std::is_signed<U>::value>> {
    constexpr bool operator()(T t, U u) { return t < 0 ? true : std::make_unsigned_t<T>(t) < u; }
};

template <typename T, typename U>
struct CmpLessImpl<T, U, std::enable_if_t<!std::is_signed<T>::value && std::is_signed<U>::value>> {
    constexpr bool operator()(T t, U u) { return u < 0 ? false : t < std::make_unsigned_t<U>(u); }
};

}  // namespace stdx
}  // namespace au



namespace au {
namespace stdx {
namespace experimental {

////////////////////////////////////////////////////////////////////////////////////////////////////
// `nonesuch`: adapted from (https://en.cppreference.com/w/cpp/experimental/nonesuch).

struct nonesuch {
    ~nonesuch() = delete;
    nonesuch(nonesuch const &) = delete;
    void operator=(nonesuch const &) = delete;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `is_detected` and friends: adapted from
// (https://en.cppreference.com/w/cpp/experimental/is_detected).

namespace detail {
template <class Default, class AlwaysVoid, template <class...> class Op, class... Args>
struct detector {
    using value_t = std::false_type;
    using type = Default;
};

template <class Default, template <class...> class Op, class... Args>
struct detector<Default, stdx::void_t<Op<Args...>>, Op, Args...> {
    using value_t = std::true_type;
    using type = Op<Args...>;
};

}  // namespace detail

template <template <class...> class Op, class... Args>
using is_detected = typename detail::detector<nonesuch, void, Op, Args...>::value_t;

template <template <class...> class Op, class... Args>
using detected_t = typename detail::detector<nonesuch, void, Op, Args...>::type;

template <class Default, template <class...> class Op, class... Args>
using detected_or = detail::detector<Default, void, Op, Args...>;

template <class Default, template <class...> class Op, class... Args>
using detected_or_t = typename detected_or<Default, Op, Args...>::type;

}  // namespace experimental
}  // namespace stdx
}  // namespace au


namespace au {
namespace stdx {

// Source: adapted from (https://en.cppreference.com/w/cpp/utility/functional/identity)
struct identity {
    template <class T>
    constexpr T &&operator()(T &&t) const noexcept {
        return std::forward<T>(t);
    }
};

}  // namespace stdx
}  // namespace au

// This file provides drop-in replacements for certain standard library function objects for
// comparison and arithmetic: `std::less<void>`, `std::plus<void>`, etc.
//
// These are _not_ intended as _fully general_ replacements.  They are _only_ intended for certain
// specific use cases in this library, where we can ensure certain preconditions are met before they
// are called.  For example, these utilities don't handle comparing signed and unsigned integral
// types, because we only ever use them in places where we've already explicitly cast our quantities
// to the same Rep.
//
// There are two main reasons we rolled our own versions instead of just using the ones from the
// standard library (as we had initially done).  First, the `<functional>` header is moderately
// expensive to include---using these alternatives could save 100 ms or more on every file.  Second,
// certain compilers (such as the Green Hills compiler) struggle with the trailing return types in,
// say, `std::less<void>::operator()`, but work correctly with our alternatives.

namespace au {
namespace detail {

//
// Comparison operators.
//

struct Equal {
    template <typename T>
    constexpr bool operator()(const T &a, const T &b) const {
        return a == b;
    }
};
constexpr auto equal = Equal{};

struct NotEqual {
    template <typename T>
    constexpr bool operator()(const T &a, const T &b) const {
        return a != b;
    }
};
constexpr auto not_equal = NotEqual{};

struct Greater {
    template <typename T>
    constexpr bool operator()(const T &a, const T &b) const {
        return a > b;
    }
};
constexpr auto greater = Greater{};

struct Less {
    template <typename T>
    constexpr bool operator()(const T &a, const T &b) const {
        return a < b;
    }
};
constexpr auto less = Less{};

struct GreaterEqual {
    template <typename T>
    constexpr bool operator()(const T &a, const T &b) const {
        return a >= b;
    }
};
constexpr auto greater_equal = GreaterEqual{};

struct LessEqual {
    template <typename T>
    constexpr bool operator()(const T &a, const T &b) const {
        return a <= b;
    }
};
constexpr auto less_equal = LessEqual{};

//
// Arithmetic operators.
//

struct Plus {
    template <typename T, typename U>
    constexpr auto operator()(const T &a, const U &b) const {
        return a + b;
    }
};
constexpr auto plus = Plus{};

struct Minus {
    template <typename T, typename U>
    constexpr auto operator()(const T &a, const U &b) const {
        return a - b;
    }
};
constexpr auto minus = Minus{};

}  // namespace detail
}  // namespace au



namespace au {
namespace detail {

template <typename PackT, typename T>
struct Prepend;
template <typename PackT, typename T>
using PrependT = typename Prepend<PackT, T>::type;

template <typename T, typename U>
struct SameTypeIgnoringCvref : std::is_same<stdx::remove_cvref_t<T>, stdx::remove_cvref_t<U>> {};

template <typename T, typename U>
constexpr bool same_type_ignoring_cvref(T, U) {
    return SameTypeIgnoringCvref<T, U>::value;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation details below.

template <template <typename...> class Pack, typename T, typename... Us>
struct Prepend<Pack<Us...>, T> {
    using type = Pack<T, Us...>;
};

}  // namespace detail
}  // namespace au



// Products of base powers are the foundation of au.  We use them for:
//
//   - The Dimension of a Unit.
//   - The Magnitude of a Unit.
//   - Making *compound* Units (products of powers, e.g., m^1 * s^(-2)).

namespace au {

// A base type B raised to an integer exponent N.
template <typename B, std::intmax_t N>
struct Pow;

// A base type B raised to a rational exponent (N/D).
template <typename B, std::intmax_t N, std::intmax_t D>
struct RatioPow;

// Type trait for the "base" of a type, interpreted as a base power.
//
// Any type can act as a base, with an implicit power of 1.  `Pow<B, N>` can represent integer
// powers of a base type `B`, and `RatioPow<B, N, D>` can represent rational powers of `B` (where
// the power is `(N/D)`).
template <typename T>
struct Base : stdx::type_identity<T> {};
template <typename T>
using BaseT = typename Base<T>::type;

// Type trait for the rational exponent of a type, interpreted as a base power.
template <typename T>
struct Exp : stdx::type_identity<std::ratio<1>> {};
template <typename T>
using ExpT = typename Exp<T>::type;

// Type trait for treating an arbitrary type as a given type of pack.
//
// This should be the identity for anything that is already a pack of this type, and otherwise
// should wrap it in this type of pack.
template <template <class... Ts> class Pack, typename T>
struct AsPack : stdx::type_identity<Pack<T>> {};
template <template <class... Ts> class Pack, typename T>
using AsPackT = typename AsPack<Pack, T>::type;

// Type trait to remove a Pack enclosing a single item.
//
// Defined only if T is Pack<Ts...> for some typelist.  Always the identity, unless sizeof...(Ts) is
// exactly 1, in which case, it returns the (sole) element.
template <template <class... Ts> class Pack, typename T>
struct UnpackIfSolo;
template <template <class... Ts> class Pack, typename T>
using UnpackIfSoloT = typename UnpackIfSolo<Pack, T>::type;

// Trait to define whether two types are in order, based on the total ordering for some pack.
//
// Each pack should individually define its desired total ordering.  For these implementations,
// prefer to inherit from LexicographicTotalOrdering (below), because it guards against the most
// common way to fail to achieve a strict total ordering (namely, by having two types which are not
// identical nevertheless compare equal).
template <template <class...> class Pack, typename A, typename B>
struct InOrderFor;

// A strict total ordering which combines strict partial orderings serially, using the first which
// distinguishes A and B.
template <typename A, typename B, template <class, class> class... Orderings>
struct LexicographicTotalOrdering;

// A (somewhat arbitrary) total ordering on _packs themselves_.
//
// Built on top of the total ordering for the _bases_ of the packs.
template <typename T, typename U>
struct InStandardPackOrder;

// Make a List of deduplicated, sorted types.
//
// The result will always be List<...>, and the elements will be sorted according to the total
// ordering for List, with duplicates removed.  It will be "flattened" in that any elements which
// are already `List<Ts...>` will be effectively replaced by `Ts...`.
//
// A precondition for `FlatDedupedTypeListT` is that any inputs which are already of type
// `List<...>`, respect the _ordering_ for `List`, with no duplicates.  Otherwise, behaviour is
// undefined.  (This precondition will automatically be satisfied if *every* instance of `List<...>`
// arises as the result of a call to `FlatDedupedTypeListT<...>`.)
template <template <class...> class List, typename... Ts>
struct FlatDedupedTypeList;
template <template <class...> class List, typename... Ts>
using FlatDedupedTypeListT = typename FlatDedupedTypeList<List, AsPackT<List, Ts>...>::type;

namespace detail {
// Express a base power in its simplest form (base alone if power is 1, or Pow if exp is integral).
template <typename T>
struct SimplifyBasePowers;
template <typename T>
using SimplifyBasePowersT = typename SimplifyBasePowers<T>::type;
}  // namespace detail

// Compute the product between two power packs.
template <template <class...> class Pack, typename... Ts>
struct PackProduct;
template <template <class...> class Pack, typename... Ts>
using PackProductT = detail::SimplifyBasePowersT<typename PackProduct<Pack, Ts...>::type>;

// Compute a rational power of a pack.
template <template <class...> class Pack, typename T, typename E>
struct PackPower;
template <template <class...> class Pack,
          typename T,
          std::intmax_t ExpNum,
          std::intmax_t ExpDen = 1>
using PackPowerT =
    detail::SimplifyBasePowersT<typename PackPower<Pack, T, std::ratio<ExpNum, ExpDen>>::type>;

// Compute the inverse of a power pack.
template <template <class...> class Pack, typename T>
using PackInverseT = PackPowerT<Pack, T, -1>;

// Compute the quotient of two power packs.
template <template <class...> class Pack, typename T, typename U>
using PackQuotientT = PackProductT<Pack, T, PackInverseT<Pack, U>>;

namespace detail {
// Pull out all of the elements in a Pack whose exponents are positive.
template <typename T>
struct NumeratorPart;
template <typename T>
using NumeratorPartT = typename NumeratorPart<T>::type;

// Pull out all of the elements in a Pack whose exponents are negative.
template <typename T>
struct DenominatorPart;
template <typename T>
using DenominatorPartT = typename DenominatorPart<T>::type;
}  // namespace detail

// A validator for a pack of Base Powers.
//
// `IsValidPack<Pack, T>::value` is `true` iff `T` is an instance of the variadic `Pack<...>`, and
// its parameters fulfill all of the appropriate type traits, namely:
//
// - `AreBasesInOrder<Pack, T>`
template <template <class...> class Pack, typename T>
struct IsValidPack;

// Assuming that `T` is an instance of `Pack<BPs...>`, validates that every consecutive pair from
// `BaseT<BPs>...` satisfies the strict total ordering `InOrderFor<Pack, ...>` for `Pack`.
template <template <class...> class Pack, typename T>
struct AreBasesInOrder;

// Assuming that `T` is an instance of `Pack<BPs...>`, validates that every consecutive pair from
// `BPs...` satisfies the strict total ordering `InOrderFor<Pack, ...>` for `Pack`.
//
// This is very similar to AreBasesInOrder, but is intended for packs that _don't_ represent
// products-of-powers.
template <template <class...> class Pack, typename T>
struct AreElementsInOrder;

// Assuming `T` is an instance of `Pack<BPs...>`, validates that `Exp<BPs>...` is always nonzero.
template <template <class...> class Pack, typename T>
struct AreAllPowersNonzero;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation details below.
////////////////////////////////////////////////////////////////////////////////////////////////////

// These forward declarations and traits go here to enable us to treat Pow and RatioPow (below) as
// full-fledged "Units".  A "Unit" is any type U where `DimT<U>` gives a valid Dimension, and
// `MagT<U>` gives a valid Magnitude.  Even though we can't define Dimension and Magnitude precisely
// in this file, we'll take advantage of the fact that we know they're going to be parameter packs.

template <typename... BPs>
struct Dimension;

template <typename... BPs>
struct Magnitude;

namespace detail {

// The default dimension, `DimT<U>`, of a type `U`, is the `::Dim` typedef (or `void` if none).
//
// Users can customize by specializing `DimImpl<U>` and setting the `type` member variable.
template <typename U>
using DimMemberT = typename U::Dim;
template <typename U>
struct DimImpl : stdx::experimental::detected_or<void, DimMemberT, U> {};
template <typename U>
using DimT = typename DimImpl<U>::type;

// The default magnitude, `MagT<U>`, of a type `U`, is the `::Mag` typedef (or `void` if none).
//
// Users can customize by specializing `MagImpl<U>` and setting the `type` member variable.
template <typename U>
using MagMemberT = typename U::Mag;
template <typename U>
struct MagImpl : stdx::experimental::detected_or<void, MagMemberT, U> {};
template <typename U>
using MagT = typename MagImpl<U>::type;

}  // namespace detail

////////////////////////////////////////////////////////////////////////////////////////////////////
// `Pow` implementation.

template <typename B, std::intmax_t N>
struct Pow {
    // TODO(#40): Clean up relationship between Dim/Mag and Pow, if compile times are OK.
    using Dim = PackPowerT<Dimension, AsPackT<Dimension, detail::DimT<B>>, N>;
    using Mag = PackPowerT<Magnitude, AsPackT<Magnitude, detail::MagT<B>>, N>;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `RatioPow` implementation.

// A base type B raised to a rational exponent (N/D).
template <typename B, std::intmax_t N, std::intmax_t D>
struct RatioPow {
    // TODO(#40): Clean up relationship between Dim/Mag and RatioPow, if compile times are OK.
    using Dim = PackPowerT<Dimension, AsPackT<Dimension, detail::DimT<B>>, N, D>;
    using Mag = PackPowerT<Magnitude, AsPackT<Magnitude, detail::MagT<B>>, N, D>;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `BaseT` implementation.

template <typename T, std::intmax_t N>
struct Base<Pow<T, N>> : stdx::type_identity<T> {};

template <typename T, std::intmax_t N, std::intmax_t D>
struct Base<RatioPow<T, N, D>> : stdx::type_identity<T> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `ExpT` implementation.

template <typename T, std::intmax_t N>
struct Exp<Pow<T, N>> : stdx::type_identity<std::ratio<N>> {};

template <typename T, std::intmax_t N, std::intmax_t D>
struct Exp<RatioPow<T, N, D>> : stdx::type_identity<std::ratio<N, D>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `AsPackT` implementation.

template <template <class... Ts> class Pack, typename... Ts>
struct AsPack<Pack, Pack<Ts...>> : stdx::type_identity<Pack<Ts...>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `UnpackIfSoloT` implementation.

// Null pack case: do not unpack.
template <template <class... Ts> class Pack>
struct UnpackIfSolo<Pack, Pack<>> : stdx::type_identity<Pack<>> {};

// Non-null pack case: unpack only if there is nothing after the head element.
template <template <class... Ts> class Pack, typename T, typename... Ts>
struct UnpackIfSolo<Pack, Pack<T, Ts...>>
    : std::conditional<(sizeof...(Ts) == 0u), T, Pack<T, Ts...>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `LexicographicTotalOrdering` implementation.

// Base case: if there is no ordering, then the inputs are not in order.
template <typename A, typename B>
struct LexicographicTotalOrdering<A, B> : std::false_type {
    // LexicographicTotalOrdering is for strict total orderings only.  If two types compare equal,
    // then they must be the same type; otherwise, we have not created a strict total ordering among
    // all types being used in some pack.
    static_assert(std::is_same<A, B>::value,
                  "Broken strict total ordering: distinct input types compare equal");
};

// Recursive case.
template <typename A,
          typename B,
          template <class, class>
          class PrimaryOrdering,
          template <class, class>
          class... Tiebreakers>
struct LexicographicTotalOrdering<A, B, PrimaryOrdering, Tiebreakers...> :

    // Short circuit for when the inputs are the same.
    //
    // This can prevent us from instantiating a tiebreaker which doesn't exist for a given type.
    std::conditional_t<
        (std::is_same<A, B>::value),
        std::false_type,

        // If A and B are properly ordered by the primary criterion, they are definitely ordered.
        std::conditional_t<(PrimaryOrdering<A, B>::value),
                           std::true_type,

                           // If B and A are properly ordered by the primary criterion, then A and B
                           // are definitely _not_ properly ordered.
                           std::conditional_t<(PrimaryOrdering<B, A>::value),
                                              std::false_type,

                                              // Fall back to the remaining orderings as
                                              // tiebreakers.
                                              LexicographicTotalOrdering<A, B, Tiebreakers...>>>> {
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `InStandardPackOrder` implementation.

namespace detail {
// Helper: check that the lead bases are in order.
template <typename T, typename U>
struct LeadBasesInOrder;
template <template <class...> class P, typename H1, typename... T1, typename H2, typename... T2>
struct LeadBasesInOrder<P<H1, T1...>, P<H2, T2...>> : InOrderFor<P, BaseT<H1>, BaseT<H2>> {};

// Helper: check that the lead exponents are in order.
template <typename T, typename U>
struct LeadExpsInOrder;
template <template <class...> class P, typename H1, typename... T1, typename H2, typename... T2>
struct LeadExpsInOrder<P<H1, T1...>, P<H2, T2...>>
    : stdx::bool_constant<(std::ratio_subtract<ExpT<H1>, ExpT<H2>>::num < 0)> {};

// Helper: apply InStandardPackOrder to tails.
template <typename T, typename U>
struct TailsInStandardPackOrder;
template <template <class...> class P, typename H1, typename... T1, typename H2, typename... T2>
struct TailsInStandardPackOrder<P<H1, T1...>, P<H2, T2...>>
    : InStandardPackOrder<P<T1...>, P<T2...>> {};
}  // namespace detail

// Base case: left pack is null.
template <template <class...> class P, typename... Ts>
struct InStandardPackOrder<P<>, P<Ts...>> : stdx::bool_constant<(sizeof...(Ts) > 0)> {};

// Base case: right pack (only) is null.
template <template <class...> class P, typename H, typename... T>
struct InStandardPackOrder<P<H, T...>, P<>> : std::false_type {};

// Recursive case: try ordering the heads, and fall back to the tails.
template <template <class...> class P, typename H1, typename... T1, typename H2, typename... T2>
struct InStandardPackOrder<P<H1, T1...>, P<H2, T2...>>
    : LexicographicTotalOrdering<P<H1, T1...>,
                                 P<H2, T2...>,
                                 detail::LeadBasesInOrder,
                                 detail::LeadExpsInOrder,
                                 detail::TailsInStandardPackOrder> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `FlatDedupedTypeListT` implementation.

// 1-ary Base case: a list with a single element is already done.
//
// (We explicitly assumed that any `List<...>` inputs would already be in sorted order.)
template <template <class...> class List, typename... Ts>
struct FlatDedupedTypeList<List, List<Ts...>> : stdx::type_identity<List<Ts...>> {};

// 2-ary base case: if we exhaust elements in the second list, the first list is the answer.
//
// (Again: this relies on the explicit assumption that any `List<...>` inputs are already in order.)
template <template <class...> class List, typename... Ts>
struct FlatDedupedTypeList<List, List<Ts...>, List<>> : stdx::type_identity<List<Ts...>> {};

// 2-ary recursive case, single-element head.
//
// This use case also serves as the core "insertion logic", inserting `T` into the proper place
// within `List<H, Ts...>`.
template <template <class...> class List, typename T, typename H, typename... Ts>
struct FlatDedupedTypeList<List, List<T>, List<H, Ts...>> :

    // If the candidate element exactly equals the head, disregard it (de-dupe!).
    std::conditional<
        (std::is_same<T, H>::value),
        List<H, Ts...>,

        // If the candidate element is strictly before the head, prepend it.
        std::conditional_t<(InOrderFor<List, T, H>::value),
                           List<T, H, Ts...>,

                           // If we're here, we know the candidate comes after the head.  So, try
                           // inserting it (recursively) in the tail, and then prepend the old Head
                           // (because we know it comes first).
                           detail::PrependT<FlatDedupedTypeListT<List, List<T>, List<Ts...>>, H>>> {
};

// 2-ary recursive case, multi-element head: insert head of second element, and recurse.
template <template <class...> class List,
          typename H1,
          typename N1,
          typename... T1,
          typename H2,
          typename... T2>
struct FlatDedupedTypeList<List, List<H1, N1, T1...>, List<H2, T2...>>
    : FlatDedupedTypeList<List,
                          // Put H2 first so we can use single-element-head case from above.
                          FlatDedupedTypeListT<List, List<H2>, List<H1, N1, T1...>>,
                          List<T2...>> {};

// N-ary case, multi-element head: peel off tail-of-head, and recurse.
//
// Note that this also handles the 2-ary case where the head list has more than one element.
template <template <class...> class List, typename L1, typename L2, typename L3, typename... Ls>
struct FlatDedupedTypeList<List, L1, L2, L3, Ls...>
    : FlatDedupedTypeList<List, FlatDedupedTypeListT<List, L1, L2>, L3, Ls...> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `PackProductT` implementation.

// 0-ary case:
template <template <class...> class Pack>
struct PackProduct<Pack> : stdx::type_identity<Pack<>> {};

// 1-ary case:
template <template <class...> class Pack, typename... Ts>
struct PackProduct<Pack, Pack<Ts...>> : stdx::type_identity<Pack<Ts...>> {};

// 2-ary Base case: two null packs.
template <template <class...> class Pack>
struct PackProduct<Pack, Pack<>, Pack<>> : stdx::type_identity<Pack<>> {};

// 2-ary Base case: only left pack is null.
template <template <class...> class Pack, typename T, typename... Ts>
struct PackProduct<Pack, Pack<>, Pack<T, Ts...>> : stdx::type_identity<Pack<T, Ts...>> {};

// 2-ary Base case: only right pack is null.
template <template <class...> class Pack, typename T, typename... Ts>
struct PackProduct<Pack, Pack<T, Ts...>, Pack<>> : stdx::type_identity<Pack<T, Ts...>> {};

namespace detail {
template <typename B, typename E1, typename E2>
struct ComputeRationalPower {
    using E = std::ratio_add<E1, E2>;
    using type = RatioPow<B, E::num, E::den>;
};
template <typename B, typename E1, typename E2>
using ComputeRationalPowerT = typename ComputeRationalPower<B, E1, E2>::type;
}  // namespace detail

// 2-ary Recursive case: two non-null packs.
template <template <class...> class P, typename H1, typename... T1, typename H2, typename... T2>
struct PackProduct<P, P<H1, T1...>, P<H2, T2...>> :

    // If the bases for H1 and H2 are in-order, prepend H1 to the product of the remainder.
    std::conditional<
        (InOrderFor<P, BaseT<H1>, BaseT<H2>>::value),
        detail::PrependT<PackProductT<P, P<T1...>, P<H2, T2...>>, H1>,

        // If the bases for H2 and H1 are in-order, prepend H2 to the product of the remainder.
        std::conditional_t<
            (InOrderFor<P, BaseT<H2>, BaseT<H1>>::value),
            detail::PrependT<PackProductT<P, P<T2...>, P<H1, T1...>>, H2>,

            // If the bases have the same position, assume they really _are_ the same (because
            // InOrderFor will verify this if it uses LexicographicTotalOrdering), and add the
            // exponents.  (If the exponents add to zero, omit the term.)
            std::conditional_t<
                (std::ratio_add<ExpT<H1>, ExpT<H2>>::num == 0),
                PackProductT<P, P<T1...>, P<T2...>>,
                detail::PrependT<PackProductT<P, P<T2...>, P<T1...>>,
                                 detail::ComputeRationalPowerT<BaseT<H1>, ExpT<H1>, ExpT<H2>>>>>> {
};

// N-ary case, N > 2: recurse.
template <template <class...> class P,
          typename... T1s,
          typename... T2s,
          typename... T3s,
          typename... Ps>
struct PackProduct<P, P<T1s...>, P<T2s...>, P<T3s...>, Ps...>
    : PackProduct<P, P<T1s...>, PackProductT<P, P<T2s...>, P<T3s...>, Ps...>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `PackPowerT` implementation.

namespace detail {
template <typename T, typename E>
using MultiplyExpFor = std::ratio_multiply<ExpT<T>, E>;
}

template <template <class...> class P, typename... Ts, typename E>
struct PackPower<P, P<Ts...>, E>
    : std::conditional<(E::num == 0),
                       P<>,
                       P<RatioPow<BaseT<Ts>,
                                  detail::MultiplyExpFor<Ts, E>::num,
                                  detail::MultiplyExpFor<Ts, E>::den>...>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `IsValidPack` implementation.

namespace detail {
template <template <class...> class Pack, typename T>
struct IsPackOf : std::false_type {};

template <template <class...> class Pack, typename... Ts>
struct IsPackOf<Pack, Pack<Ts...>> : std::true_type {};
}  // namespace detail

template <template <class...> class Pack, typename T>
struct IsValidPack : stdx::conjunction<detail::IsPackOf<Pack, T>,
                                       AreBasesInOrder<Pack, T>,
                                       AreAllPowersNonzero<Pack, T>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `AreElementsInOrder` implementation.

template <template <class...> class Pack>
struct AreElementsInOrder<Pack, Pack<>> : std::true_type {};

template <template <class...> class Pack, typename T>
struct AreElementsInOrder<Pack, Pack<T>> : std::true_type {};

template <template <class...> class Pack, typename T1, typename T2, typename... Ts>
struct AreElementsInOrder<Pack, Pack<T1, T2, Ts...>>
    : stdx::conjunction<InOrderFor<Pack, T1, T2>, AreElementsInOrder<Pack, Pack<T2, Ts...>>> {};

namespace detail {

constexpr bool all_true() { return true; }

template <typename... Predicates>
constexpr bool all_true(Predicates &&...values) {
    // The reason we bother to make an array is so that we can iterate over it.
    const bool value_array[] = {values...};

    for (auto i = 0u; i < sizeof...(Predicates); ++i) {
        if (!value_array[i]) {
            return false;
        }
    }

    return true;
}
}  // namespace detail

////////////////////////////////////////////////////////////////////////////////////////////////////
// `AreBasesInOrder` implementation.

template <template <class...> class Pack, typename... Ts>
struct AreBasesInOrder<Pack, Pack<Ts...>> : AreElementsInOrder<Pack, Pack<BaseT<Ts>...>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `AreAllPowersNonzero` implementation.

template <template <class...> class Pack, typename... Ts>
struct AreAllPowersNonzero<Pack, Pack<Ts...>>
    : stdx::bool_constant<detail::all_true((ExpT<Ts>::num != 0)...)> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `SimplifyBasePowersT` implementation.

namespace detail {
// To simplify an individual base power, by default, do nothing.
template <typename T>
struct SimplifyBasePower : stdx::type_identity<T> {};
template <typename T>
using SimplifyBasePowerT = typename SimplifyBasePower<T>::type;

// To simplify an integer power of a base, give the base alone if the exponent is 1; otherwise, do
// nothing.
template <typename B, std::intmax_t N>
struct SimplifyBasePower<Pow<B, N>> : std::conditional<(N == 1), B, Pow<B, N>> {};

// To simplify a rational power of a base, simplify the integer power if the exponent is an integer
// (i.e., if its denominator is 1); else, do nothing.
template <typename B, std::intmax_t N, std::intmax_t D>
struct SimplifyBasePower<RatioPow<B, N, D>>
    : std::conditional<(D == 1), SimplifyBasePowerT<Pow<B, N>>, RatioPow<B, N, D>> {};

// To simplify the base powers in a pack, give the pack with each base power simplified.
template <template <class...> class Pack, typename... BPs>
struct SimplifyBasePowers<Pack<BPs...>> : stdx::type_identity<Pack<SimplifyBasePowerT<BPs>...>> {};
}  // namespace detail

////////////////////////////////////////////////////////////////////////////////////////////////////
// `NumeratorPartT` and `DenominatorPartT` implementation.

namespace detail {
template <template <class...> class Pack>
struct NumeratorPart<Pack<>> : stdx::type_identity<Pack<>> {};

template <template <class...> class Pack, typename Head, typename... Tail>
struct NumeratorPart<Pack<Head, Tail...>>
    : std::conditional<(ExpT<Head>::num > 0),
                       PackProductT<Pack, Pack<Head>, NumeratorPartT<Pack<Tail...>>>,
                       NumeratorPartT<Pack<Tail...>>> {};

template <template <class...> class Pack, typename... Ts>
struct DenominatorPart<Pack<Ts...>> : NumeratorPart<PackInverseT<Pack, Pack<Ts...>>> {};
}  // namespace detail

}  // namespace au


namespace au {

template <typename... BPs>
struct Dimension {
    // Having separate `static_assert` instances for the individual conditions produces more
    // readable errors if we fail.
    static_assert(AreAllPowersNonzero<Dimension, Dimension<BPs...>>::value,
                  "All powers must be nonzero");
    static_assert(AreBasesInOrder<Dimension, Dimension<BPs...>>::value,
                  "Bases must be listed in ascending order");

    // We also want to use the "full" validity check.  This should be equivalent to the above
    // conditions, but if we add more conditions later, we want them to get picked up here
    // automatically.
    static_assert(IsValidPack<Dimension, Dimension<BPs...>>::value, "Ill-formed Dimension");
};

// Define readable operations for product, quotient, power, inverse on Dimensions.
template <typename... BPs>
using DimProductT = PackProductT<Dimension, BPs...>;
template <typename T, std::intmax_t ExpNum, std::intmax_t ExpDen = 1>
using DimPowerT = PackPowerT<Dimension, T, ExpNum, ExpDen>;
template <typename T, typename U>
using DimQuotientT = PackQuotientT<Dimension, T, U>;
template <typename T>
using DimInverseT = PackInverseT<Dimension, T>;

template <typename... BP1s, typename... BP2s>
constexpr auto operator*(Dimension<BP1s...>, Dimension<BP2s...>) {
    return DimProductT<Dimension<BP1s...>, Dimension<BP2s...>>{};
}

template <typename... BP1s, typename... BP2s>
constexpr auto operator/(Dimension<BP1s...>, Dimension<BP2s...>) {
    return DimQuotientT<Dimension<BP1s...>, Dimension<BP2s...>>{};
}

// Roots and powers for Dimension instances.
template <std::intmax_t N, typename... BPs>
constexpr DimPowerT<Dimension<BPs...>, N> pow(Dimension<BPs...>) {
    return {};
}
template <std::intmax_t N, typename... BPs>
constexpr DimPowerT<Dimension<BPs...>, 1, N> root(Dimension<BPs...>) {
    return {};
}

template <typename... Dims>
struct CommonDimension;
template <typename... Dims>
using CommonDimensionT = typename CommonDimension<Dims...>::type;

template <typename... BaseDims>
struct CommonDimension<Dimension<BaseDims...>> : stdx::type_identity<Dimension<BaseDims...>> {};
template <typename Head, typename... Tail>
struct CommonDimension<Head, Tail...> : CommonDimension<Tail...> {
    static_assert(std::is_same<Head, CommonDimensionT<Tail...>>::value,
                  "Common dimension only defined when all dimensions are identical");
};

namespace base_dim {

template <int64_t I>
struct BaseDimension {
    static constexpr int64_t base_dim_index = I;
};
template <int64_t I>
constexpr int64_t BaseDimension<I>::base_dim_index;

template <typename T, typename U>
struct OrderByBaseDimIndex : stdx::bool_constant<(T::base_dim_index < U::base_dim_index)> {};

struct Length : BaseDimension<-99> {};
struct Mass : BaseDimension<-98> {};
struct Time : BaseDimension<-97> {};
struct Current : BaseDimension<-96> {};
struct Temperature : BaseDimension<-95> {};
struct Angle : BaseDimension<-94> {};
struct Information : BaseDimension<-93> {};
struct AmountOfSubstance : BaseDimension<-92> {};
struct LuminousIntensity : BaseDimension<-91> {};

}  // namespace base_dim

template <typename A, typename B>
struct InOrderFor<Dimension, A, B>
    : LexicographicTotalOrdering<A, B, base_dim::OrderByBaseDimIndex> {};

// The types we want to expose to the rest of the library internals are the full-fledged Dimensions,
// not the Base Dimensions, because Dimensions are easier to work with (we can take products,
// quotients, powers, etc.).
using Length = Dimension<base_dim::Length>;
using Mass = Dimension<base_dim::Mass>;
using Time = Dimension<base_dim::Time>;
using Current = Dimension<base_dim::Current>;
using Temperature = Dimension<base_dim::Temperature>;
using Angle = Dimension<base_dim::Angle>;
using Information = Dimension<base_dim::Information>;
using AmountOfSubstance = Dimension<base_dim::AmountOfSubstance>;
using LuminousIntensity = Dimension<base_dim::LuminousIntensity>;

}  // namespace au



// "Magnitude" is a collection of templated types, representing positive real numbers.
//
// The key design goal is to support products and rational powers _exactly_, including for many
// irrational numbers, such as Pi, or sqrt(2).
//
// Even though there is only one possible value for each type, we encourage users to use these
// values wherever possible, because they interact correctly via standard `*`, `/`, `==`, and `!=`
// operations, and this leads to more readable code.

namespace au {

template <typename... BPs>
struct Magnitude {
    // Having separate `static_assert` instances for the individual conditions produces more
    // readable errors if we fail.
    static_assert(AreAllPowersNonzero<Magnitude, Magnitude<BPs...>>::value,
                  "All powers must be nonzero");
    static_assert(AreBasesInOrder<Magnitude, Magnitude<BPs...>>::value,
                  "Bases must be listed in ascending order");

    // We also want to use the "full" validity check.  This should be equivalent to the above
    // conditions, but if we add more conditions later, we want them to get picked up here
    // automatically.
    static_assert(IsValidPack<Magnitude, Magnitude<BPs...>>::value, "Ill-formed Magnitude");
};

// Define readable operations for product, quotient, power, inverse on Magnitudes.
template <typename... BPs>
using MagProductT = PackProductT<Magnitude, BPs...>;
template <typename T, std::intmax_t ExpNum, std::intmax_t ExpDen = 1>
using MagPowerT = PackPowerT<Magnitude, T, ExpNum, ExpDen>;
template <typename T, typename U>
using MagQuotientT = PackQuotientT<Magnitude, T, U>;
template <typename T>
using MagInverseT = PackInverseT<Magnitude, T>;

// A helper function to create a Magnitude from an integer constant.
template <std::size_t N>
constexpr auto mag();

// A base type for prime numbers.
template <std::uintmax_t N>
struct Prime {
    static_assert(detail::is_prime(N), "Prime<N> requires that N is prime");

    static constexpr std::uintmax_t value() { return N; }
};

// A base type for pi.
struct Pi {
    // The reason we define this manually, rather than using something like `M_PIl`, is because the
    // latter is not available on certain architectures.  We do test against `M_PIl`.  Those tests
    // are not run on architectures that don't support `M_PIl`, but as long as they are run on any
    // architectures at all, that's enough to give confidence in this value.
    //
    // Source for value: http://www.pi-world-ranking-list.com/lists/details/hogg.html
    static constexpr long double value() { return 3.14159265358979323846264338327950288419716939L; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Define the lexicographic ordering of bases for Magnitude.

namespace detail {
template <typename T, typename U>
struct OrderByValue : stdx::bool_constant<(T::value() < U::value())> {};
}  // namespace detail

template <typename A, typename B>
struct InOrderFor<Magnitude, A, B> : LexicographicTotalOrdering<A, B, detail::OrderByValue> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Type trait based interface for Magnitude.

template <typename MagT>
struct IntegerPartImpl;
template <typename MagT>
using IntegerPartT = typename IntegerPartImpl<MagT>::type;

template <typename MagT>
struct NumeratorImpl;
template <typename MagT>
using NumeratorT = typename NumeratorImpl<MagT>::type;

template <typename MagT>
using DenominatorT = NumeratorT<MagInverseT<MagT>>;

template <typename MagT>
struct IsRational
    : std::is_same<MagT,
                   MagQuotientT<IntegerPartT<NumeratorT<MagT>>, IntegerPartT<DenominatorT<MagT>>>> {
};

template <typename MagT>
struct IsInteger : std::is_same<MagT, IntegerPartT<MagT>> {};

// The "common magnitude" of two Magnitudes is the largest Magnitude that evenly divides both.
//
// This is possible only if the quotient of the inputs is rational.  If it's not, then the "common
// magnitude" is one that is related to both inputs, and symmetrical under a change in order (to
// fulfill the requirements of a `std::common_type` specialization).
template <typename... Ms>
struct CommonMagnitude;
template <typename... Ms>
using CommonMagnitudeT = typename CommonMagnitude<Ms...>::type;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Value based interface for Magnitude.

static constexpr auto ONE = Magnitude<>{};
static constexpr auto PI = Magnitude<Pi>{};

template <typename... BP1s, typename... BP2s>
constexpr auto operator*(Magnitude<BP1s...>, Magnitude<BP2s...>) {
    return MagProductT<Magnitude<BP1s...>, Magnitude<BP2s...>>{};
}

template <typename... BP1s, typename... BP2s>
constexpr auto operator/(Magnitude<BP1s...>, Magnitude<BP2s...>) {
    return MagQuotientT<Magnitude<BP1s...>, Magnitude<BP2s...>>{};
}

template <int E, typename... BPs>
constexpr auto pow(Magnitude<BPs...>) {
    return MagPowerT<Magnitude<BPs...>, E>{};
}

template <int N, typename... BPs>
constexpr auto root(Magnitude<BPs...>) {
    return MagPowerT<Magnitude<BPs...>, 1, N>{};
}

template <typename... BP1s, typename... BP2s>
constexpr auto operator==(Magnitude<BP1s...>, Magnitude<BP2s...>) {
    return std::is_same<Magnitude<BP1s...>, Magnitude<BP2s...>>::value;
}

template <typename... BP1s, typename... BP2s>
constexpr auto operator!=(Magnitude<BP1s...> m1, Magnitude<BP2s...> m2) {
    return !(m1 == m2);
}

template <typename... BPs>
constexpr auto integer_part(Magnitude<BPs...>) {
    return IntegerPartT<Magnitude<BPs...>>{};
}

template <typename... BPs>
constexpr auto numerator(Magnitude<BPs...>) {
    return NumeratorT<Magnitude<BPs...>>{};
}

template <typename... BPs>
constexpr auto denominator(Magnitude<BPs...>) {
    return DenominatorT<Magnitude<BPs...>>{};
}

template <typename... BPs>
constexpr bool is_rational(Magnitude<BPs...>) {
    return IsRational<Magnitude<BPs...>>::value;
}

template <typename... BPs>
constexpr bool is_integer(Magnitude<BPs...>) {
    return IsInteger<Magnitude<BPs...>>::value;
}

// Get the value of this Magnitude in a "traditional" numeric type T.
//
// If T is an integral type, then the Magnitude must be integral as well.
template <typename T, typename... BPs>
constexpr T get_value(Magnitude<BPs...>);

// Value-based interface around CommonMagnitude.
template <typename... Ms>
constexpr auto common_magnitude(Ms...) {
    return CommonMagnitudeT<Ms...>{};
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation details below.
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
// `mag<N>()` implementation.

namespace detail {

// Helper to perform prime factorization.
template <std::uintmax_t N>
struct PrimeFactorization;
template <std::uintmax_t N>
using PrimeFactorizationT = typename PrimeFactorization<N>::type;

// Base case: factorization of 1.
template <>
struct PrimeFactorization<1u> : stdx::type_identity<Magnitude<>> {};

template <std::uintmax_t N>
struct PrimeFactorization {
    static_assert(N > 0, "Can only factor positive integers");

    static constexpr std::uintmax_t first_base = find_first_factor(N);
    static constexpr std::uintmax_t first_power = multiplicity(first_base, N);
    static constexpr std::uintmax_t remainder = N / int_pow(first_base, first_power);

    using type =
        MagProductT<Magnitude<Pow<Prime<first_base>, first_power>>, PrimeFactorizationT<remainder>>;
};

}  // namespace detail

template <std::size_t N>
constexpr auto mag() {
    return detail::PrimeFactorizationT<N>{};
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// `integer_part()` implementation.

template <typename B, typename P>
struct IntegerPartOfBasePower : stdx::type_identity<Magnitude<>> {};

// Raise B to the largest natural number power which won't exceed (N/D), or 0 if there isn't one.
template <std::uintmax_t B, std::intmax_t N, std::intmax_t D>
struct IntegerPartOfBasePower<Prime<B>, std::ratio<N, D>>
    : stdx::type_identity<MagPowerT<Magnitude<Prime<B>>, ((N >= D) ? (N / D) : 0)>> {};

template <typename... BPs>
struct IntegerPartImpl<Magnitude<BPs...>>
    : stdx::type_identity<
          MagProductT<typename IntegerPartOfBasePower<BaseT<BPs>, ExpT<BPs>>::type...>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `numerator()` implementation.

template <typename... BPs>
struct NumeratorImpl<Magnitude<BPs...>>
    : stdx::type_identity<
          MagProductT<std::conditional_t<(ExpT<BPs>::num > 0), Magnitude<BPs>, Magnitude<>>...>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `get_value<T>(Magnitude)` implementation.

namespace detail {

enum class MagRepresentationOutcome {
    OK,
    ERR_NON_INTEGER_IN_INTEGER_TYPE,
    ERR_INVALID_ROOT,
    ERR_CANNOT_FIT,
};

template <typename T>
struct MagRepresentationOrError {
    MagRepresentationOutcome outcome;

    // Only valid/meaningful if `outcome` is `OK`.
    T value = {0};
};

// The widest arithmetic type in the same category.
//
// Used for intermediate computations.
template <typename T>
using Widen = std::conditional_t<
    std::is_arithmetic<T>::value,
    std::conditional_t<std::is_floating_point<T>::value,
                       long double,
                       std::conditional_t<std::is_signed<T>::value, std::intmax_t, std::uintmax_t>>,
    T>;

template <typename T>
constexpr MagRepresentationOrError<T> checked_int_pow(T base, std::uintmax_t exp) {
    MagRepresentationOrError<T> result = {MagRepresentationOutcome::OK, T{1}};
    while (exp > 0u) {
        if (exp % 2u == 1u) {
            if (base > std::numeric_limits<T>::max() / result.value) {
                return MagRepresentationOrError<T>{MagRepresentationOutcome::ERR_CANNOT_FIT};
            }
            result.value *= base;
        }

        exp /= 2u;

        if (base > std::numeric_limits<T>::max() / base) {
            return (exp == 0u)
                       ? result
                       : MagRepresentationOrError<T>{MagRepresentationOutcome::ERR_CANNOT_FIT};
        }
        base *= base;
    }
    return result;
}

template <typename T>
constexpr MagRepresentationOrError<T> root(T x, std::uintmax_t n) {
    // The "zeroth root" would be mathematically undefined.
    if (n == 0) {
        return {MagRepresentationOutcome::ERR_INVALID_ROOT};
    }

    // The "first root" is trivial.
    if (n == 1) {
        return {MagRepresentationOutcome::OK, x};
    }

    // We only support nontrivial roots of floating point types.
    if (!std::is_floating_point<T>::value) {
        return {MagRepresentationOutcome::ERR_NON_INTEGER_IN_INTEGER_TYPE};
    }

    // Handle negative numbers: only odd roots are allowed.
    if (x < 0) {
        if (n % 2 == 0) {
            return {MagRepresentationOutcome::ERR_INVALID_ROOT};
        } else {
            const auto negative_result = root(-x, n);
            if (negative_result.outcome != MagRepresentationOutcome::OK) {
                return {negative_result.outcome};
            }
            return {MagRepresentationOutcome::OK, static_cast<T>(-negative_result.value)};
        }
    }

    // Handle special cases of zero and one.
    if (x == 0 || x == 1) {
        return {MagRepresentationOutcome::OK, x};
    }

    // Handle numbers bewtween 0 and 1.
    if (x < 1) {
        const auto inverse_result = root(T{1} / x, n);
        if (inverse_result.outcome != MagRepresentationOutcome::OK) {
            return {inverse_result.outcome};
        }
        return {MagRepresentationOutcome::OK, static_cast<T>(T{1} / inverse_result.value)};
    }

    //
    // At this point, error conditions are finished, and we can proceed with the "core" algorithm.
    //

    // Always use `long double` for intermediate computations.  We don't ever expect people to be
    // calling this at runtime, so we want maximum accuracy.
    long double lo = 1.0;
    long double hi = x;

    // Do a binary search to find the closest value such that `checked_int_pow` recovers the input.
    //
    // Because we know `n > 1`, and `x > 1`, and x^n is monotonically increasing, we know that
    // `checked_int_pow(lo, n) < x < checked_int_pow(hi, n)`.  We will preserve this as an
    // invariant.
    while (lo < hi) {
        long double mid = lo + (hi - lo) / 2;

        auto result = checked_int_pow(mid, n);

        if (result.outcome != MagRepresentationOutcome::OK) {
            return {result.outcome};
        }

        // Early return if we get lucky with an exact answer.
        if (result.value == x) {
            return {MagRepresentationOutcome::OK, static_cast<T>(mid)};
        }

        // Check for stagnation.
        if (mid == lo || mid == hi) {
            break;
        }

        // Preserve the invariant that `checked_int_pow(lo, n) < x < checked_int_pow(hi, n)`.
        if (result.value < x) {
            lo = mid;
        } else {
            hi = mid;
        }
    }

    // Pick whichever one gets closer to the target.
    const auto lo_diff = x - checked_int_pow(lo, n).value;
    const auto hi_diff = checked_int_pow(hi, n).value - x;
    return {MagRepresentationOutcome::OK, static_cast<T>(lo_diff < hi_diff ? lo : hi)};
}

template <typename T, std::intmax_t N, std::uintmax_t D, typename B>
constexpr MagRepresentationOrError<Widen<T>> base_power_value(B base) {
    if (N < 0) {
        const auto inverse_result = base_power_value<T, -N, D>(base);
        if (inverse_result.outcome != MagRepresentationOutcome::OK) {
            return inverse_result;
        }
        return {
            MagRepresentationOutcome::OK,
            Widen<T>{1} / inverse_result.value,
        };
    }

    const auto power_result =
        checked_int_pow(static_cast<Widen<T>>(base), static_cast<std::uintmax_t>(N));
    if (power_result.outcome != MagRepresentationOutcome::OK) {
        return {power_result.outcome};
    }
    return (D > 1) ? root(power_result.value, D) : power_result;
}

template <typename T, std::size_t N>
constexpr MagRepresentationOrError<T> product(const MagRepresentationOrError<T> (&values)[N]) {
    for (const auto &x : values) {
        if (x.outcome != MagRepresentationOutcome::OK) {
            return x;
        }
    }

    T result{1};
    for (const auto &x : values) {
        if ((x.value > 1) && (result > std::numeric_limits<T>::max() / x.value)) {
            return {MagRepresentationOutcome::ERR_CANNOT_FIT};
        }
        result *= x.value;
    }
    return {MagRepresentationOutcome::OK, result};
}

template <std::size_t N>
constexpr bool all(const bool (&values)[N]) {
    for (const auto &x : values) {
        if (!x) {
            return false;
        }
    }
    return true;
}

template <typename Target, typename Enable = void>
struct SafeCastingChecker {
    template <typename T>
    constexpr bool operator()(T x) {
        return stdx::cmp_less_equal(std::numeric_limits<Target>::lowest(), x) &&
               stdx::cmp_greater_equal(std::numeric_limits<Target>::max(), x);
    }
};

template <typename Target>
struct SafeCastingChecker<Target, std::enable_if_t<std::is_integral<Target>::value>> {
    template <typename T>
    constexpr bool operator()(T x) {
        return std::is_integral<T>::value &&
               stdx::cmp_less_equal(std::numeric_limits<Target>::lowest(), x) &&
               stdx::cmp_greater_equal(std::numeric_limits<Target>::max(), x);
    }
};

template <typename T, typename InputT>
constexpr bool safe_to_cast_to(InputT x) {
    return SafeCastingChecker<T>{}(x);
}

template <typename T, typename... BPs>
constexpr MagRepresentationOrError<T> get_value_result(Magnitude<BPs...>) {
    // Representing non-integer values in integral types is something we never plan to support.
    constexpr bool REPRESENTING_NON_INTEGER_IN_INTEGRAL_TYPE =
        stdx::conjunction<std::is_integral<T>, stdx::negation<IsInteger<Magnitude<BPs...>>>>::value;
    if (REPRESENTING_NON_INTEGER_IN_INTEGRAL_TYPE) {
        return {MagRepresentationOutcome::ERR_NON_INTEGER_IN_INTEGER_TYPE};
    }

    // Force the expression to be evaluated in a constexpr context.
    constexpr auto widened_result =
        product({base_power_value<T, ExpT<BPs>::num, static_cast<std::uintmax_t>(ExpT<BPs>::den)>(
            BaseT<BPs>::value())...});

    if ((widened_result.outcome != MagRepresentationOutcome::OK) ||
        !safe_to_cast_to<T>(widened_result.value)) {
        return {MagRepresentationOutcome::ERR_CANNOT_FIT};
    }

    return {MagRepresentationOutcome::OK, static_cast<T>(widened_result.value)};
}

// This simple overload avoids edge cases with creating and passing zero-sized arrays.
template <typename T>
constexpr MagRepresentationOrError<T> get_value_result(Magnitude<>) {
    return {MagRepresentationOutcome::OK, static_cast<T>(1)};
}
}  // namespace detail

template <typename T, typename... BPs>
constexpr bool representable_in(Magnitude<BPs...> m) {
    using namespace detail;

    return get_value_result<T>(m).outcome == MagRepresentationOutcome::OK;
}

template <typename T, typename... BPs>
constexpr T get_value(Magnitude<BPs...> m) {
    using namespace detail;

    constexpr auto result = get_value_result<T>(m);

    static_assert(result.outcome != MagRepresentationOutcome::ERR_NON_INTEGER_IN_INTEGER_TYPE,
                  "Cannot represent non-integer in integral destination type");
    static_assert(result.outcome != MagRepresentationOutcome::ERR_INVALID_ROOT,
                  "Could not compute root for rational power of base");
    static_assert(result.outcome != MagRepresentationOutcome::ERR_CANNOT_FIT,
                  "Value outside range of destination type");

    static_assert(result.outcome == MagRepresentationOutcome::OK, "Unknown error occurred");
    return result.value;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// `CommonMagnitude` implementation.

namespace detail {
// Helper: prepend a base power, but only if the Exp is negative.
template <typename BP, typename MagT>
struct PrependIfExpNegative;
template <typename BP, typename MagT>
using PrependIfExpNegativeT = typename PrependIfExpNegative<BP, MagT>::type;
template <typename BP, typename... Ts>
struct PrependIfExpNegative<BP, Magnitude<Ts...>>
    : std::conditional<(ExpT<BP>::num < 0), Magnitude<BP, Ts...>, Magnitude<Ts...>> {};

// If M is (N/D), DenominatorPartT<M> is D; we want 1/D.
template <typename M>
using NegativePowers = MagInverseT<DenominatorPartT<M>>;
}  // namespace detail

// 1-ary case: identity.
template <typename M>
struct CommonMagnitude<M> : stdx::type_identity<M> {};

// 2-ary base case: both Magnitudes null.
template <>
struct CommonMagnitude<Magnitude<>, Magnitude<>> : stdx::type_identity<Magnitude<>> {};

// 2-ary base case: only left Magnitude is null.
template <typename Head, typename... Tail>
struct CommonMagnitude<Magnitude<>, Magnitude<Head, Tail...>>
    : stdx::type_identity<detail::NegativePowers<Magnitude<Head, Tail...>>> {};

// 2-ary base case: only right Magnitude is null.
template <typename Head, typename... Tail>
struct CommonMagnitude<Magnitude<Head, Tail...>, Magnitude<>>
    : stdx::type_identity<detail::NegativePowers<Magnitude<Head, Tail...>>> {};

// 2-ary recursive case: two non-null Magnitudes.
template <typename H1, typename... T1, typename H2, typename... T2>
struct CommonMagnitude<Magnitude<H1, T1...>, Magnitude<H2, T2...>> :

    // If the bases for H1 and H2 are in-order, prepend H1-if-negative to the remainder.
    std::conditional<
        (InOrderFor<Magnitude, BaseT<H1>, BaseT<H2>>::value),
        detail::PrependIfExpNegativeT<H1, CommonMagnitudeT<Magnitude<T1...>, Magnitude<H2, T2...>>>,

        // If the bases for H2 and H1 are in-order, prepend H2-if-negative to the remainder.
        std::conditional_t<
            (InOrderFor<Magnitude, BaseT<H2>, BaseT<H1>>::value),
            detail::PrependIfExpNegativeT<H2,
                                          CommonMagnitudeT<Magnitude<T2...>, Magnitude<H1, T1...>>>,

            // If we got here, the bases must be the same.  (We can assume that `InOrderFor` does
            // proper checking to guard against equivalent-but-not-identical bases, which would
            // violate total ordering.)
            std::conditional_t<
                (std::ratio_subtract<ExpT<H1>, ExpT<H2>>::num < 0),
                detail::PrependT<CommonMagnitudeT<Magnitude<T1...>, Magnitude<T2...>>, H1>,
                detail::PrependT<CommonMagnitudeT<Magnitude<T1...>, Magnitude<T2...>>, H2>>>> {};

// N-ary case: recurse.
template <typename M1, typename M2, typename... Tail>
struct CommonMagnitude<M1, M2, Tail...> : CommonMagnitude<M1, CommonMagnitudeT<M2, Tail...>> {};

// Zero is always ignored.
template <typename M>
struct CommonMagnitude<M, Zero> : stdx::type_identity<M> {};
template <typename M>
struct CommonMagnitude<Zero, M> : stdx::type_identity<M> {};
template <>
struct CommonMagnitude<Zero, Zero> : stdx::type_identity<Zero> {};

}  // namespace  au



// This file exists to analyze one single calculation: `x * N / D`, where `x` is
// some integral type, and `N` and `D` are the numerator and denominator of a
// rational magnitude (and hence, are automatically in lowest terms),
// represented in that same type.  We want to answer one single question: will
// this calculation overflow at any stage?
//
// Importantly, we need to produce correct answers even when `N` and/or `D`
// _cannot be represented_ in that type (because they would overflow).  We also
// need to handle subtleties around integer promotion, where the type of `x * x`
// can be different from the type of `x` when those types are small.
//
// The goal for the final solution we produce is to be as fast and efficient as
// the best such function that an expert C++ engineer could produce by hand, for
// every combination of integral type and numerator and denominator magnitudes.

namespace au {
namespace detail {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// `PromotedType<T>` is the result type for arithmetic operations involving `T`.  Of course, this is
// normally just `T`, but integer promotion for small integral types can change this.
//
template <typename T>
struct PromotedTypeImpl {
    using type = decltype(std::declval<T>() * std::declval<T>());

    static_assert(std::is_same<type, typename PromotedTypeImpl<type>::type>::value,
                  "We explicitly assume that promoted types are not again promotable");
};
template <typename T>
using PromotedType = typename PromotedTypeImpl<T>::type;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// `clamp_to_range_of<T>(x)` returns `x` if it is in the range of `T`, and otherwise returns the
// maximum value representable in `T` if `x` is too large, or the minimum value representable in `T`
// if `x` is too small.
//

template <typename T, typename U>
constexpr T clamp_to_range_of(U x) {
    return stdx::cmp_greater(x, std::numeric_limits<T>::max())
               ? std::numeric_limits<T>::max()
               : (stdx::cmp_less(x, std::numeric_limits<T>::lowest())
                      ? std::numeric_limits<T>::lowest()
                      : static_cast<T>(x));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// `is_known_to_be_less_than_one(MagT)` is true if the magnitude `MagT` is purely rational; its
// numerator is representable in `std::uintmax_t`; and, it is less than 1.
//

template <typename... BPs>
constexpr bool is_known_to_be_less_than_one(Magnitude<BPs...> m) {
    using MagT = Magnitude<BPs...>;
    static_assert(is_rational(m), "Magnitude must be rational");

    constexpr auto num_result = get_value_result<std::uintmax_t>(numerator(MagT{}));
    static_assert(num_result.outcome == MagRepresentationOutcome::OK,
                  "Magnitude must be representable in std::uintmax_t");

    constexpr auto den_result = get_value_result<std::uintmax_t>(denominator(MagT{}));
    static_assert(
        den_result.outcome == MagRepresentationOutcome::OK ||
            den_result.outcome == MagRepresentationOutcome::ERR_CANNOT_FIT,
        "Magnitude must either be representable in std::uintmax_t, or fail due to overflow");

    return den_result.outcome == MagRepresentationOutcome::OK ? num_result.value < den_result.value
                                                              : true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// `MaxNonOverflowingValue<T, MagT>` is the maximum value of type `T` that can have `MagT` applied
// as numerator-and-denominator without overflowing.  We require that `T` is some integral
// arithmetic type, and that `MagT` is a rational magnitude that is neither purely integral nor
// purely inverse-integral.
//
// If you are trying to understand these helpers, we suggest starting at the bottom with
// `MaxNonOverflowingValue`, and reading upwards.
//

//
// Branch based on whether `MagT` is less than 1.
//
template <typename T, typename MagT, bool IsMagLessThanOne>
struct MaxNonOverflowingValueImplWhenNumFits;

// If `MagT` is less than 1, then we only need to check for the limiting value where the _numerator
// multiplication step alone_ would overflow.
template <typename T, typename MagT>
struct MaxNonOverflowingValueImplWhenNumFits<T, MagT, true> {
    using P = PromotedType<T>;

    static constexpr T value() {
        return clamp_to_range_of<T>(std::numeric_limits<P>::max() /
                                    get_value<P>(numerator(MagT{})));
    }
};

// If `MagT` is greater than 1, then we have two opportunities for overflow: the numerator
// multiplication step can overflow the promoted type; or, the denominator division step can fail to
// restore it to the original type's range.
template <typename T, typename MagT>
struct MaxNonOverflowingValueImplWhenNumFits<T, MagT, false> {
    using P = PromotedType<T>;

    static constexpr T value() {
        constexpr auto num = get_value<P>(numerator(MagT{}));
        constexpr auto den = get_value<P>(denominator(MagT{}));
        constexpr auto t_max = std::numeric_limits<T>::max();
        constexpr auto p_max = std::numeric_limits<P>::max();
        constexpr auto limit_to_avoid = (den > p_max / t_max) ? p_max : t_max * den;
        return clamp_to_range_of<T>(limit_to_avoid / num);
    }
};

//
// Branch based on whether the numerator of `MagT` can fit in the promoted type of `T`.
//
template <typename T, typename MagT, MagRepresentationOutcome NumOutcome>
struct MaxNonOverflowingValueImpl;

// If the numerator fits in the promoted type of `T`, delegate further based on whether the
// denominator is bigger.
template <typename T, typename MagT>
struct MaxNonOverflowingValueImpl<T, MagT, MagRepresentationOutcome::OK>
    : MaxNonOverflowingValueImplWhenNumFits<T, MagT, is_known_to_be_less_than_one(MagT{})> {};

// If `MagT` can't be represented in the promoted type of `T`, then the result is 0.
template <typename T, typename MagT>
struct MaxNonOverflowingValueImpl<T, MagT, MagRepresentationOutcome::ERR_CANNOT_FIT> {
    static constexpr T value() { return T{0}; }
};

template <typename T, typename MagT>
struct ValidateTypeAndMagnitude {
    static_assert(std::is_integral<T>::value, "Only designed for integral types");
    static_assert(is_rational(MagT{}), "Magnitude must be rational");
    static_assert(!is_integer(MagT{}), "Magnitude must not be purely integral");
    static_assert(!is_integer(inverse(MagT{})), "Magnitude must not be purely inverse-integral");
};

template <typename T, typename MagT>
struct MaxNonOverflowingValue
    : ValidateTypeAndMagnitude<T, MagT>,
      MaxNonOverflowingValueImpl<T,
                                 MagT,
                                 get_value_result<PromotedType<T>>(numerator(MagT{})).outcome> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// `MinNonOverflowingValue<T, MagT>` is the minimum (i.e., most-negative) value of type `T` that can
// have `MagT` applied as numerator-and-denominator without overflowing (i.e., becoming too-negative
// to represent).  We require that `T` is some integral arithmetic type, and that `MagT` is a
// rational magnitude that is neither purely integral nor purely inverse-integral.
//
// If you are trying to understand these helpers, we suggest starting at the bottom with
// `MinNonOverflowingValue`, and reading upwards.
//

//
// Branch based on whether `MagT` is less than 1.
//
template <typename T, typename MagT, bool IsMagLessThanOne>
struct MinNonOverflowingValueImplWhenNumFits;

// If `MagT` is less than 1, then we only need to check for the limiting value where the _numerator
// multiplication step alone_ would overflow.
template <typename T, typename MagT>
struct MinNonOverflowingValueImplWhenNumFits<T, MagT, true> {
    using P = PromotedType<T>;

    static constexpr T value() {
        return clamp_to_range_of<T>(std::numeric_limits<P>::lowest() /
                                    get_value<P>(numerator(MagT{})));
    }
};

// If `MagT` is greater than 1, then we have two opportunities for overflow: the numerator
// multiplication step can overflow the promoted type; or, the denominator division step can fail to
// restore it to the original type's range.
template <typename T, typename MagT>
struct MinNonOverflowingValueImplWhenNumFits<T, MagT, false> {
    using P = PromotedType<T>;

    static constexpr T value() {
        constexpr auto num = get_value<P>(numerator(MagT{}));
        constexpr auto den = get_value<P>(denominator(MagT{}));
        constexpr auto t_min = std::numeric_limits<T>::lowest();
        constexpr auto p_min = std::numeric_limits<P>::lowest();
        constexpr auto limit_to_avoid = (den > p_min / t_min) ? p_min : t_min * den;
        return clamp_to_range_of<T>(limit_to_avoid / num);
    }
};

//
// Branch based on whether the denominator of `MagT` can fit in the promoted type of `T`.
//
template <typename T, typename MagT, MagRepresentationOutcome NumOutcome>
struct MinNonOverflowingValueImpl;

// If the numerator fits in the promoted type of `T`, delegate further based on whether the
// denominator is bigger.
template <typename T, typename MagT>
struct MinNonOverflowingValueImpl<T, MagT, MagRepresentationOutcome::OK>
    : MinNonOverflowingValueImplWhenNumFits<T, MagT, is_known_to_be_less_than_one(MagT{})> {};

// If the numerator can't be represented in the promoted type of `T`, then the result is 0.
template <typename T, typename MagT>
struct MinNonOverflowingValueImpl<T, MagT, MagRepresentationOutcome::ERR_CANNOT_FIT> {
    static constexpr T value() { return T{0}; }
};

template <typename T, typename MagT>
struct MinNonOverflowingValue
    : ValidateTypeAndMagnitude<T, MagT>,
      MinNonOverflowingValueImpl<T,
                                 MagT,
                                 get_value_result<PromotedType<T>>(numerator(MagT{})).outcome> {
    static_assert(std::is_signed<T>::value, "Only designed for signed types");
    static_assert(std::is_signed<PromotedType<T>>::value,
                  "We assume the promoted type is also signed");
};

}  // namespace detail
}  // namespace au


namespace au {

// A "unit" is any type which has:
// - a member typedef `Dim`, which is a valid Dimension; and,
// - a member typedef `Mag`, which is a valid Magnitude.
//
// These can be accessed by traits `detail::DimT` and `detail::MagT`, respectively.  The detail
// namespace is meant to discourage _end users_ from accessing these concepts directly.  For
// example, we don't want end users to ask _which dimension_ a Unit has.  We'd rather they ask
// whether it is the _same_ as some other unit.  (It's also meaningful to ask whether it is
// dimensionless.)  And we certainly don't want end users to try to reason about "the magnitude" of
// a Unit, since this is totally meaningless; rather, we want them to ask about the _relative_
// magnitude with another unit of _the same dimension_.

// A UnitImpl is one easy way (although not the only way) to make a "Unit".
template <typename D, typename M = Magnitude<>>
struct UnitImpl {
    using Dim = D;
    using Mag = M;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Printable labels for units.

// A printable label to indicate the unit for human readers.
//
// To name a unit explicitly, specialize this class template for the unit's type.  For any unit not
// manually labeled, we provide a default label so that this template is always defined.
//
// Valid ways to define the label include a C-style const char array, or a StringConstant<N>.
template <typename Unit>
struct UnitLabel;

// A sizeof()-compatible API to get the label for a unit.
template <typename Unit>
constexpr const auto &unit_label(Unit = Unit{});

// Default label for a unit which hasn't been manually labeled yet.
//
// The dummy template parameter exists to enable `au` to be a header-only library.
template <typename T = void>
struct DefaultUnitLabel {
    static constexpr const char value[] = "[UNLABELED UNIT]";
};
template <typename T>
constexpr const char DefaultUnitLabel<T>::value[];

namespace detail {
// To preserve support for C++14, we need to _name the type_ of the member variable.  However, the
// `StringConstant` template produces a different type for every length, and that length depends on
// _both_ the prefix _and_ the unit label.
//
// To minimize friction as much as possible, we create this alias, which computes the type we need
// for a given unit and prefix-length.
//
// While clunky, this approach is at least robust against errors.  If the user supplies the wrong
// prefix length, it will fail to compile, because there is no assignment operator between
// `StringConstant` instances of different lengths.
template <std::size_t ExtensionStrlen, typename... Us>
using ExtendedLabel = StringConstant<concatenate(unit_label<Us>()...).size() + ExtensionStrlen>;
}  // namespace detail

////////////////////////////////////////////////////////////////////////////////////////////////////
// Type traits.

// Type trait to detect whether a type fulfills our definition of a "Unit".
template <typename T>
struct IsUnit : stdx::conjunction<IsValidPack<Dimension, detail::DimT<T>>,
                                  IsValidPack<Magnitude, detail::MagT<T>>> {};

// Type trait to detect whether two Units have the same Dimension.
template <typename... Us>
struct HasSameDimension;

// Type trait to detect whether two Units are quantity-equivalent.
//
// In this library, Units are "quantity-equivalent" exactly when they have the same Dimension and
// Magnitude.  Quantity instances whose Units are quantity-equivalent can be freely interconverted
// with each other.
template <typename U1, typename U2>
struct AreUnitsQuantityEquivalent;

// Type trait to detect whether two Units are point-equivalent.
//
// In this library, Units are "point-equivalent" exactly when they are quantity-equivalent (see
// above), _and_ they have the same origin.  QuantityPoint instances whose Units are
// point-equivalent can be freely interconverted with each other.
template <typename U1, typename U2>
struct AreUnitsPointEquivalent;

// Type trait to detect whether U is a Unit which is dimensionless.
template <typename U>
struct IsDimensionless : std::is_same<detail::DimT<U>, Dimension<>> {};

// Type trait to detect whether a Unit is "quantity-equivalent" to "the unitless unit".
//
// The "unitless unit" is a dimensionless unit of Magnitude 1 (as opposed to, say, other
// dimensionless units such as Percent).
template <typename U>
struct IsUnitlessUnit
    : stdx::conjunction<IsDimensionless<U>, std::is_same<detail::MagT<U>, Magnitude<>>> {};

// A Magnitude representing the ratio of two same-dimensioned units.
//
// Useful in doing unit conversions.
template <typename U1, typename U2>
struct UnitRatio : stdx::type_identity<MagQuotientT<detail::MagT<U1>, detail::MagT<U2>>> {
    static_assert(HasSameDimension<U1, U2>::value,
                  "Can only compute ratio of same-dimension units");
};
template <typename U1, typename U2>
using UnitRatioT = typename UnitRatio<U1, U2>::type;

// Some units have an "origin".  This is not meaningful by itself, but its difference w.r.t. the
// "origin" of another unit of the same Dimension _is_ meaningful.  This type trait provides access
// to that difference.
template <typename U1, typename U2>
struct OriginDisplacement;

template <typename U>
struct AssociatedUnit : stdx::type_identity<U> {};
template <typename U>
using AssociatedUnitT = typename AssociatedUnit<U>::type;

// `CommonUnitT`: the largest unit that evenly divides all input units.
//
// A specialization will only exist if all input types are units.
//
// If the inputs are units, but their Dimensions aren't all identical, then the request is
// ill-formed and we will produce a hard error.
//
// It may happen that the input units have the same Dimension, but there is no unit which evenly
// divides them (because some pair of input units has an irrational quotient).  In this case, there
// is no uniquely defined answer, but the program should still produce _some_ answer.  We guarantee
// that the result is associative, and symmetric under any reordering of the input units.  The
// specific implementation choice will be driven by convenience and simplicity.
template <typename... Us>
struct ComputeCommonUnit;
template <typename... Us>
using CommonUnitT = typename ComputeCommonUnit<Us...>::type;

// `CommonPointUnitT`: the largest-magnitude, highest-origin unit which is "common" to the units of
// a collection of `QuantityPoint` instances.
//
// The key goal to keep in mind is that for a `QuantityPoint` of any unit `U` in `Us...`, converting
// its value to the common point-unit should involve only:
//
//   - multiplication by a _positive integer_
//   - addition of a _non-negative integer_
//
// This helps us support the widest range of Rep types (in particular, unsigned integers).
//
// As with `CommonUnitT`, this isn't always possible: in particular, we can't do this for units with
// irrational relative magnitudes or origin displacements.  However, we still provide _some_ answer,
// which is consistent with the above policy whenever it's achievable, and produces reasonable
// results in all other cases.
//
// A specialization will only exist if the inputs are all units, and will exist but produce a hard
// error if any two input units have different Dimensions.  We also strive to keep the result
// associative, and symmetric under interchange of any inputs.
template <typename... Us>
struct ComputeCommonPointUnit;
template <typename... Us>
using CommonPointUnitT = typename ComputeCommonPointUnit<Us...>::type;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Type traits (instance-based interface).

// `is_unit(T)`: check whether this value is an instance of some Unit type.
template <typename T>
constexpr bool is_unit(T) {
    return IsUnit<T>::value;
}

// `fits_in_unit_slot(T)`: check whether this value is valid for a unit slot.
template <typename T>
constexpr bool fits_in_unit_slot(T) {
    return IsUnit<AssociatedUnitT<T>>::value;
}

// Check whether the units associated with these objects have the same Dimension.
template <typename... Us>
constexpr bool has_same_dimension(Us...) {
    return HasSameDimension<AssociatedUnitT<Us>...>::value;
}

// Check whether two Unit types are exactly quantity-equivalent.
template <typename U1, typename U2>
constexpr bool are_units_quantity_equivalent(U1, U2) {
    return AreUnitsQuantityEquivalent<AssociatedUnitT<U1>, AssociatedUnitT<U2>>::value;
}

// Check whether two Unit types are exactly point-equivalent.
template <typename U1, typename U2>
constexpr bool are_units_point_equivalent(U1, U2) {
    return AreUnitsPointEquivalent<AssociatedUnitT<U1>, AssociatedUnitT<U2>>::value;
}

// Check whether this value is an instance of a dimensionless Unit.
template <typename U>
constexpr bool is_dimensionless(U) {
    return IsDimensionless<AssociatedUnitT<U>>::value;
}

// Type trait to detect whether a Unit is "the unitless unit".
template <typename U>
constexpr bool is_unitless_unit(U) {
    return IsUnitlessUnit<AssociatedUnitT<U>>::value;
}

// A Magnitude representing the ratio of two same-dimensioned units.
//
// Useful in doing unit conversions.
template <typename U1, typename U2>
constexpr UnitRatioT<AssociatedUnitT<U1>, AssociatedUnitT<U2>> unit_ratio(U1, U2) {
    return {};
}

template <typename U1, typename U2>
constexpr auto origin_displacement(U1, U2) {
    return OriginDisplacement<AssociatedUnitT<U1>, AssociatedUnitT<U2>>::value();
}

template <typename U>
constexpr auto associated_unit(U) {
    return AssociatedUnitT<U>{};
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Unit arithmetic traits: products, powers, and derived operations.

// A Unit, scaled by some factor.
//
// Retains all of the member variables and typedefs of the existing Unit, except that the
// `detail::MagT` trait is appropriately scaled, and the unit label is erased.
//
// NOTE: This strategy will lead to long chains of inherited types when we scale a unit multiple
// times (say, going from Meters -> Centi<Meters> -> Inches -> Feet -> Miles).  What's more, each
// element in this chain yields _two_ types: one for the named opaque typedef (e.g., `Feet`), and
// one for the anonymous scaled unit (e.g., `Inches * mag<12>()`).  We explicitly assume that this
// will not cause any performance problems, because these should all be empty classes anyway.  If we
// find out we're mistaken, we'll need to revisit this idea.
template <typename Unit, typename ScaleFactor>
struct ScaledUnit : Unit {
    static_assert(IsValidPack<Magnitude, ScaleFactor>::value,
                  "Can only scale by a Magnitude<...> type");
    using Dim = detail::DimT<Unit>;
    using Mag = MagProductT<detail::MagT<Unit>, ScaleFactor>;

    // We must ensure we don't give this unit the same label as the unscaled version!
    //
    // Later on, we could try generating a new label by "pretty printing" the scale factor.
    static constexpr auto &label = DefaultUnitLabel<void>::value;
};

// Type template to hold the product of powers of Units.
template <typename... UnitPows>
struct UnitProduct {
    using Dim = DimProductT<detail::DimT<UnitPows>...>;
    using Mag = MagProductT<detail::MagT<UnitPows>...>;
};

// Helper to make a canonicalized product of units.
//
// On the input side, we treat every input unit as a UnitProduct.  Once we get our final result, we
// simplify it using `UnpackIfSoloT`.  (The motivation is that we don't want to return, say,
// `UnitProduct<Meters>`; we'd rather just return `Meters`.)
template <typename... UnitPows>
using UnitProductT =
    UnpackIfSoloT<UnitProduct, PackProductT<UnitProduct, AsPackT<UnitProduct, UnitPows>...>>;

// Raise a Unit to a (possibly rational) Power.
template <typename U, std::intmax_t ExpNum, std::intmax_t ExpDen = 1>
using UnitPowerT =
    UnpackIfSoloT<UnitProduct, PackPowerT<UnitProduct, AsPackT<UnitProduct, U>, ExpNum, ExpDen>>;

// Compute the inverse of a unit.
template <typename U>
using UnitInverseT = UnitPowerT<U, -1>;

// Compute the quotient of two units.
template <typename U1, typename U2>
using UnitQuotientT = UnitProductT<U1, UnitInverseT<U2>>;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Unit arithmetic on _instances_ of Units and/or Magnitudes.

// Scale this Unit by multiplying by a Magnitude.
template <typename U, typename = std::enable_if_t<IsUnit<U>::value>, typename... BPs>
constexpr ScaledUnit<U, Magnitude<BPs...>> operator*(U, Magnitude<BPs...>) {
    return {};
}

// Scale this Unit by dividing by a Magnitude.
template <typename U, typename = std::enable_if_t<IsUnit<U>::value>, typename... BPs>
constexpr ScaledUnit<U, MagInverseT<Magnitude<BPs...>>> operator/(U, Magnitude<BPs...>) {
    return {};
}

// Compute the product of two unit instances.
template <typename U1,
          typename U2,
          typename = std::enable_if_t<stdx::conjunction<IsUnit<U1>, IsUnit<U2>>::value>>
constexpr UnitProductT<U1, U2> operator*(U1, U2) {
    return {};
}

// Compute the quotient of two unit instances.
template <typename U1,
          typename U2,
          typename = std::enable_if_t<stdx::conjunction<IsUnit<U1>, IsUnit<U2>>::value>>
constexpr UnitQuotientT<U1, U2> operator/(U1, U2) {
    return {};
}

// Raise a Unit to an integral power.
template <std::intmax_t Exp, typename U, typename = std::enable_if_t<IsUnit<U>::value>>
constexpr UnitPowerT<U, Exp> pow(U) {
    return {};
}

// Take the Root (of some integral degree) of a Unit.
template <std::uintmax_t Deg, typename U, typename = std::enable_if_t<IsUnit<U>::value>>
constexpr UnitPowerT<U, 1, Deg> root(U) {
    return {};
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Miscellaneous interfaces.

// An instance which lets us refer to a unit by its singular name.
//
// To use this, whenever you define a new unit (e.g., `struct Meters`), follow it up with a line
// like the following:
//
//     constexpr auto meter = SingularNameFor<Meters>{};
//
// This is just to help us write grammatically natural code.  Examples:
//
//   - `torque.in(newton * meters)`
//                ^^^^^^
//   - `speed.as(miles / hour)`
//                       ^^^^
template <typename Unit>
struct SingularNameFor {

    // Multiplying `SingularNameFor` instances enables compound units such as:
    // `radians / (meter * second)`.
    template <typename OtherUnit>
    constexpr auto operator*(SingularNameFor<OtherUnit>) const {
        return SingularNameFor<UnitProductT<Unit, OtherUnit>>{};
    }
};

template <int Exp, typename Unit>
constexpr auto pow(SingularNameFor<Unit>) {
    return SingularNameFor<UnitPowerT<Unit, Exp>>{};
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation details below
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
// `OriginDisplacement` implementation.

namespace detail {
// Callable type trait for the default origin of a unit: choose ZERO.
struct ZeroValue {
    static constexpr Zero value() { return Zero{}; }
};

template <typename U>
using OriginMemberType = decltype(U::origin());

// If any unit U has an explicit origin member, then treat that as its origin.
template <typename U>
struct OriginMember {
    static constexpr const OriginMemberType<U> value() { return U::origin(); }
};

template <typename U>
struct OriginOf : std::conditional_t<stdx::experimental::is_detected<OriginMemberType, U>::value,
                                     OriginMember<U>,
                                     ZeroValue> {};

template <typename T, typename U>
struct ValueDifference {
    static constexpr auto value() { return T::value() - U::value(); }
};
}  // namespace detail

// Why this conditional, instead of just using `ValueDifference` unconditionally?  The use case is
// somewhat subtle.  Without it, we would still deduce a displacement _numerically_ equal to 0, but
// it would be stored in specific _units_.  For example, for Celsius, the displacement would be "0
// millikelvins" rather than a generic ZERO.  This has implications for type deduction.  It means
// that, e.g., the following would fail!
//
//   celsius_pt(20).in(celsius_pt);
//
// The reason it would fail is because under the hood, we'd be subtracting a `QuantityI32<Celsius>`
// from a `QuantityI32<Milli<Kelvins>>`, yielding a result expressed in millikelvins for what should
// be an integer number of degrees Celsius.  True, that result happens to have a _value_ of 0... but
// values don't affect overload resolution!
//
// Using ZeroValue when the origins are equal fixes this problem, by expressing the "zero-ness" in
// the _type_.
template <typename U1, typename U2>
struct OriginDisplacement
    : std::conditional_t<detail::OriginOf<U1>::value() == detail::OriginOf<U2>::value(),
                         detail::ZeroValue,
                         detail::ValueDifference<detail::OriginOf<U2>, detail::OriginOf<U1>>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `HasSameDimension` implementation.

template <typename U>
struct HasSameDimension<U> : std::true_type {};

template <typename U1, typename U2, typename... Us>
struct HasSameDimension<U1, U2, Us...>
    : stdx::conjunction<std::is_same<detail::DimT<U1>, detail::DimT<U2>>,
                        HasSameDimension<U2, Us...>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `AreUnitsQuantityEquivalent` implementation.

namespace detail {
// We don't want to advertise this utility, because "same magnitude" is meaningless unless the units
// also have the same dimension.
template <typename U1, typename U2>
struct HasSameMagnitude : std::is_same<detail::MagT<U1>, detail::MagT<U2>> {};
}  // namespace detail

template <typename U1, typename U2>
struct AreUnitsQuantityEquivalent
    : stdx::conjunction<HasSameDimension<U1, U2>, detail::HasSameMagnitude<U1, U2>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `AreUnitsPointEquivalent` implementation.

namespace detail {
template <typename U1, typename U2>
struct HasSameOrigin : stdx::bool_constant<(OriginDisplacement<U1, U2>::value() == ZERO)> {};
}  // namespace detail

template <typename U1, typename U2>
struct AreUnitsPointEquivalent
    : stdx::conjunction<AreUnitsQuantityEquivalent<U1, U2>, detail::HasSameOrigin<U1, U2>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `CommonUnit` helper implementation.

// This exists to be the "named type" for the common unit of a bunch of input units.
//
// To be well-formed, the units must be listed in the same order every time.  End users cannot be
// responsible for this; thus, they should never name this type directly.  Rather, they should name
// the `CommonUnitT` alias, which will handle the canonicalization.
template <typename... Us>
struct CommonUnit {
    static_assert(AreElementsInOrder<CommonUnit, CommonUnit<Us...>>::value,
                  "Elements must be listed in ascending order");
    static_assert(HasSameDimension<Us...>::value,
                  "Common unit only meaningful if units have same dimension");

    using Dim = CommonDimensionT<detail::DimT<Us>...>;
    using Mag = CommonMagnitudeT<detail::MagT<Us>...>;
};

template <typename A, typename B>
struct InOrderFor<CommonUnit, A, B> : InOrderFor<UnitProduct, A, B> {};

namespace detail {
// This machinery searches a unit list for one that "matches" a target unit.
//
// If none do, it will produce the target unit.

// Generic template.
template <template <class, class> class Matcher,
          typename TargetUnit,
          typename UnitList = TargetUnit>
struct FirstMatchingUnit;

// Base case for an empty list: the target unit is the best match.
template <template <class, class> class Matcher,
          typename TargetUnit,
          template <class...>
          class List>
struct FirstMatchingUnit<Matcher, TargetUnit, List<>> : stdx::type_identity<TargetUnit> {};

// Recursive case for a non-empty list: return head if it matches, or else recurse.
template <template <class, class> class Matcher,
          typename TargetUnit,
          template <class...>
          class List,
          typename H,
          typename... Ts>
struct FirstMatchingUnit<Matcher, TargetUnit, List<H, Ts...>>
    : std::conditional_t<Matcher<TargetUnit, H>::value,
                         stdx::type_identity<H>,
                         FirstMatchingUnit<Matcher, TargetUnit, List<Ts...>>> {};

}  // namespace detail

template <typename... Us>
using ComputeCommonUnitImpl = FlatDedupedTypeListT<CommonUnit, Us...>;

template <typename... Us>
struct ComputeCommonUnit
    : detail::FirstMatchingUnit<AreUnitsQuantityEquivalent, ComputeCommonUnitImpl<Us...>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `CommonPointUnitT` helper implementation.

namespace detail {

// For equal origins expressed in different units, we can compare the values in their native units
// as a way to decide which unit has the biggest Magnitude.  Bigger Magnitude, smaller value.  (We
// could have tried to assess the Magnitude directly, but this method works better with Zero, and we
// will often encounter Zero when dealing with origins.)
//
// This will be used as a tiebreaker for different origin types.  (For example, the origin of
// Celsius may be represented as Centikelvins or Millikelvins, and we want Centikelvins to "win"
// because it will result in smaller multiplications.)
template <typename T>
constexpr auto get_value_in_native_unit(const T &t) {
    return t.in(T::unit);
}

// If the input is "0", then its value _in any unit_ is 0.
constexpr auto get_value_in_native_unit(const Zero &) { return 0; }

// The common origin of a collection of units is the smallest origin.
//
// We try to keep the result symmetric under reordering of the inputs.
template <typename... Us>
struct CommonOrigin;

template <typename U>
struct CommonOrigin<U> : OriginOf<U> {};

template <typename Head, typename... Tail>
struct CommonOrigin<Head, Tail...> :
    // If the new value is strictly less than the common-so-far, then it wins, so choose it.
    std::conditional_t<
        (OriginOf<Head>::value() < CommonOrigin<Tail...>::value()),
        OriginOf<Head>,

        // If the new value is strictly greater than the common-so-far, it's worse, so skip it.
        std::conditional_t<
            (OriginOf<Head>::value() > CommonOrigin<Tail...>::value()),
            CommonOrigin<Tail...>,

            // If we're here, the origins represent the same _quantity_, but may be expressed in
            // different _units_.  We'd like the biggest unit, since it leads to the smallest
            // multiplications.  For equal quantities, "biggest unit" is equivalent to "smallest
            // value", so we compare the values.
            std::conditional_t<(get_value_in_native_unit(OriginOf<Head>::value()) <
                                get_value_in_native_unit(CommonOrigin<Tail...>::value())),
                               OriginOf<Head>,
                               CommonOrigin<Tail...>>>> {};

// MagTypeT<T> gives some measure of the size of the unit for this "quantity-alike" type.
//
// Zero acts like a quantity in this context, and we treat it as if its unit's Magnitude is Zero.
// This is specifically done for the `CommonPointUnit` implementation; there is no guarantee that
template <typename QuantityOrZero>
struct MagType : stdx::type_identity<MagT<typename QuantityOrZero::Unit>> {};
template <typename QuantityOrZero>
using MagTypeT = typename MagType<stdx::remove_cvref_t<QuantityOrZero>>::type;
template <>
struct MagType<Zero> : stdx::type_identity<Zero> {};

}  // namespace detail

// This exists to be the "named type" for the common unit of a bunch of input units.
//
// To be well-formed, the units must be listed in the same order every time.  End users cannot be
// responsible for this; thus, they should never name this type directly.  Rather, they should name
// the `CommonPointUnitT` alias, which will handle the canonicalization.
template <typename... Us>
struct CommonPointUnit {
    static_assert(AreElementsInOrder<CommonPointUnit, CommonPointUnit<Us...>>::value,
                  "Elements must be listed in ascending order");
    static_assert(HasSameDimension<Us...>::value,
                  "Common unit only meaningful if units have same dimension");

    // We need to store the origin member inside of a type, so that it will act "just enough" like a
    // unit to let us use `OriginDisplacement`.  (We'll link to this nested type's origin member for
    // our own origin member.)
    struct TypeHoldingCommonOrigin {
        using OriginT = decltype(detail::CommonOrigin<Us...>::value());
        static constexpr OriginT origin() { return detail::CommonOrigin<Us...>::value(); }
    };
    static constexpr auto origin() { return TypeHoldingCommonOrigin::origin(); }

    // This handles checking that all the dimensions are the same.  It's what lets us reason in
    // terms of pure Magnitudes below, whereas usually this kind of reasoning is meaningless.
    using Dim = CommonDimensionT<detail::DimT<Us>...>;

    // Now, for Magnitude reasoning.  `OriginDisplacementMagnitude` tells us how finely grained we
    // are forced to split our Magnitude to handle the additive displacements from the common
    // origin.  It might be `Zero` if there is no such constraint (which would mean all the units
    // have the _same_ origin).
    using OriginDisplacementMagnitude = CommonMagnitudeT<
        detail::MagTypeT<decltype(OriginDisplacement<TypeHoldingCommonOrigin, Us>::value())>...>;

    // The final Magnitude is just what it would have been before, except that we also take the
    // results of `OriginDisplacementMagnitude` into account.
    using Mag = CommonMagnitudeT<detail::MagT<Us>..., OriginDisplacementMagnitude>;
};

template <typename A, typename B>
struct InOrderFor<CommonPointUnit, A, B> : InOrderFor<UnitProduct, A, B> {};

template <typename... Us>
using ComputeCommonPointUnitImpl = FlatDedupedTypeListT<CommonPointUnit, Us...>;

template <typename... Us>
struct ComputeCommonPointUnit
    : detail::FirstMatchingUnit<AreUnitsPointEquivalent, ComputeCommonPointUnitImpl<Us...>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `UnitLabel` implementation.

namespace detail {
template <std::size_t N>
constexpr auto as_char_array(const char (&x)[N]) -> const char (&)[N] {
    return x;
}

template <std::size_t N>
constexpr auto as_char_array(const StringConstant<N> &x) -> const char (&)[N + 1] {
    return x.char_array();
}

template <typename Unit>
using HasLabel = decltype(Unit::label);

// Implementation for units that do have a label.
template <typename T>
struct LabelRef {
    static constexpr auto &value = T::label;
};

// Utility for labeling a unit raised to some power.
template <typename ExpLabel, typename Unit>
struct PowerLabeler {
    using LabelT = ExtendedLabel<ExpLabel::value().size() + 1, Unit>;
    static constexpr LabelT value = join_by("^", unit_label<Unit>(), ExpLabel::value());
};
template <typename ExpLabeler, typename Unit>
constexpr typename PowerLabeler<ExpLabeler, Unit>::LabelT PowerLabeler<ExpLabeler, Unit>::value;

// Utility to generate the exponent label for a Pow.
template <std::intmax_t N>
struct ExpLabelForPow {
    static constexpr auto value() { return parens_if<(N < 0)>(IToA<N>::value); }
};

// Utility to generate the exponent label for a RatioPow.
template <std::intmax_t N, std::intmax_t D>
struct ExpLabelForRatioPow {
    static constexpr auto value() {
        return concatenate("(", IToA<N>::value, "/", IToA<D>::value, ")");
    }
};

enum class ParensPolicy {
    OMIT,
    ADD_IF_MULITPLE,
};

template <typename T, ParensPolicy Policy = ParensPolicy::ADD_IF_MULITPLE>
struct CompoundLabel;
template <typename... Us, ParensPolicy Policy>
struct CompoundLabel<UnitProduct<Us...>, Policy> {
    static constexpr auto value() {
        constexpr bool add_parens =
            (Policy == ParensPolicy::ADD_IF_MULITPLE) && (sizeof...(Us) > 1);
        return parens_if<add_parens>(join_by(" * ", unit_label<Us>()...));
    }
};

// Labeler for a quotient of products-of-Units: general case.
//
// The dummy template parameter exists to enable `au` to be a header-only library.
template <typename N, typename D, typename T = void>
struct QuotientLabeler {
    using LabelT =
        StringConstant<CompoundLabel<N>::value().size() + CompoundLabel<D>::value().size() + 3>;
    static constexpr LabelT value =
        join_by(" / ", CompoundLabel<N>::value(), CompoundLabel<D>::value());
};
template <typename N, typename D, typename T>
constexpr typename QuotientLabeler<N, D, T>::LabelT QuotientLabeler<N, D, T>::value;

// Special case for denominator of 1.
template <typename N, typename T>
struct QuotientLabeler<N, UnitProduct<>, T> {
    using LabelT = StringConstant<CompoundLabel<N, ParensPolicy::OMIT>::value().size()>;
    static constexpr LabelT value = CompoundLabel<N, ParensPolicy::OMIT>::value();
};
template <typename N, typename T>
constexpr typename QuotientLabeler<N, UnitProduct<>, T>::LabelT
    QuotientLabeler<N, UnitProduct<>, T>::value;

// Special case for numerator of 1.
template <typename D, typename T>
struct QuotientLabeler<UnitProduct<>, D, T> {
    using LabelT = StringConstant<CompoundLabel<D>::value().size() + 4>;
    static constexpr LabelT value = concatenate("1 / ", CompoundLabel<D>::value());
};
template <typename D, typename T>
constexpr typename QuotientLabeler<UnitProduct<>, D, T>::LabelT
    QuotientLabeler<UnitProduct<>, D, T>::value;

// Special case for numerator _and_ denominator of 1 (null product).
template <typename T>
struct QuotientLabeler<UnitProduct<>, UnitProduct<>, T> {
    static constexpr const char value[] = "";
};
template <typename T>
constexpr const char QuotientLabeler<UnitProduct<>, UnitProduct<>, T>::value[];
}  // namespace detail

// Unified implementation.
template <typename Unit>
struct UnitLabel
    : std::conditional_t<stdx::experimental::is_detected<detail::HasLabel, Unit>::value,
                         detail::LabelRef<Unit>,
                         DefaultUnitLabel<void>> {};

// Implementation for Pow.
template <typename Unit, std::intmax_t N>
struct UnitLabel<Pow<Unit, N>> : detail::PowerLabeler<detail::ExpLabelForPow<N>, Unit> {};

// Implementation for RatioPow.
template <typename Unit, std::intmax_t N, std::intmax_t D>
struct UnitLabel<RatioPow<Unit, N, D>>
    : detail::PowerLabeler<detail::ExpLabelForRatioPow<N, D>, Unit> {};

// Implementation for UnitProduct: split into positive and negative powers.
template <typename... Us>
struct UnitLabel<UnitProduct<Us...>>
    : detail::QuotientLabeler<detail::NumeratorPartT<UnitProduct<Us...>>,
                              detail::DenominatorPartT<UnitProduct<Us...>>,
                              void> {};

// Implementation for CommonUnit: unite constituent labels.
template <typename... Us>
struct UnitLabel<CommonUnit<Us...>> {
    using LabelT = detail::ExtendedLabel<5 + 2 * (sizeof...(Us) - 1), Us...>;
    static constexpr LabelT value =
        detail::concatenate("COM[", detail::join_by(", ", unit_label(Us{})...), "]");
};
template <typename... Us>
constexpr typename UnitLabel<CommonUnit<Us...>>::LabelT UnitLabel<CommonUnit<Us...>>::value;

// Implementation for CommonPointUnit: unite constituent labels.
template <typename... Us>
struct UnitLabel<CommonPointUnit<Us...>> {
    using LabelT = detail::ExtendedLabel<8 + 2 * (sizeof...(Us) - 1), Us...>;
    static constexpr LabelT value =
        detail::concatenate("COM_PT[", detail::join_by(", ", unit_label(Us{})...), "]");
};
template <typename... Us>
constexpr
    typename UnitLabel<CommonPointUnit<Us...>>::LabelT UnitLabel<CommonPointUnit<Us...>>::value;

template <typename Unit>
constexpr const auto &unit_label(Unit) {
    return detail::as_char_array(UnitLabel<AssociatedUnitT<Unit>>::value);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// `UnitProduct` implementation.
//
// It's just a standard pack product, so all we need to do is carefully define the total ordering.

namespace detail {
template <typename A, typename B>
struct OrderByDim : InStandardPackOrder<DimT<A>, DimT<B>> {};

template <typename A, typename B>
struct OrderByMag : InStandardPackOrder<MagT<A>, MagT<B>> {};

// OrderAsUnitProduct<A, B> can only be true if both A and B are unit products, _and_ they are in
// the standard pack order for unit products.  This default case handles the usual case where either
// A or B (or both) is not a UnitProduct<...> in the first place.
template <typename A, typename B>
struct OrderAsUnitProduct : std::false_type {};

// This specialization handles the non-trivial case, where we do have two UnitProduct instances.
template <typename... U1s, typename... U2s>
struct OrderAsUnitProduct<UnitProduct<U1s...>, UnitProduct<U2s...>>
    : InStandardPackOrder<UnitProduct<U1s...>, UnitProduct<U2s...>> {};

template <typename A, typename B>
struct OrderByOrigin : stdx::bool_constant<(OriginDisplacement<A, B>::value() < ZERO)> {};

// "Unit avoidance" is a tiebreaker for quantity-equivalent units.  Anonymous units, such as
// `UnitImpl<...>`, `ScaledUnit<...>`, and `UnitProduct<...>`, are more "avoidable" than units which
// are none of these, because the latter are likely explicitly named and thus more user-facing.  The
// relative ordering among these built-in template types is probably less important than the fact
// that there _is_ a relative ordering among them (because we need to have a strict total ordering).
template <typename T>
struct UnitAvoidance : std::integral_constant<int, 0> {};

template <typename A, typename B>
struct OrderByUnitAvoidance
    : stdx::bool_constant<(UnitAvoidance<A>::value < UnitAvoidance<B>::value)> {};

template <typename... Ts>
struct UnitAvoidance<UnitProduct<Ts...>> : std::integral_constant<int, 1> {};

template <typename... Ts>
struct UnitAvoidance<UnitImpl<Ts...>> : std::integral_constant<int, 2> {};

template <typename... Ts>
struct UnitAvoidance<ScaledUnit<Ts...>> : std::integral_constant<int, 3> {};

template <typename B, std::intmax_t N>
struct UnitAvoidance<Pow<B, N>> : std::integral_constant<int, 4> {};

template <typename B, std::intmax_t N, std::intmax_t D>
struct UnitAvoidance<RatioPow<B, N, D>> : std::integral_constant<int, 5> {};

template <typename... Us>
struct UnitAvoidance<CommonUnit<Us...>> : std::integral_constant<int, 6> {};

template <typename... Us>
struct UnitAvoidance<CommonPointUnit<Us...>> : std::integral_constant<int, 7> {};
}  // namespace detail

template <typename A, typename B>
struct InOrderFor<UnitProduct, A, B> : LexicographicTotalOrdering<A,
                                                                  B,
                                                                  detail::OrderByUnitAvoidance,
                                                                  detail::OrderByDim,
                                                                  detail::OrderByMag,
                                                                  detail::OrderByOrigin,
                                                                  detail::OrderAsUnitProduct> {};

}  // namespace au



namespace au {

// Check that this particular Magnitude won't cause this specific value to overflow its type.
template <typename Rep, typename... BPs>
constexpr bool can_scale_without_overflow(Magnitude<BPs...> m, Rep value) {
    // Scales that shrink don't cause overflow.
    if (get_value<double>(m) <= 1.0) {
        (void)value;
        return true;
    } else {
        return std::numeric_limits<Rep>::max() / get_value<Rep>(m) >= value;
    }
}

namespace detail {
// Chosen so as to allow populating a `QuantityI32<Hertz>` with an input in MHz.
constexpr auto OVERFLOW_THRESHOLD = 2'147;

// This wrapper for `can_scale_without_overflow<...>(..., OVERFLOW_THRESHOLD)` can prevent an
// instantiation via short-circuiting, speeding up compile times.
template <typename Rep, typename ScaleFactor>
struct CanScaleThresholdWithoutOverflow
    : stdx::conjunction<
          stdx::bool_constant<stdx::in_range<Rep>(OVERFLOW_THRESHOLD)>,
          stdx::bool_constant<can_scale_without_overflow<Rep>(ScaleFactor{}, OVERFLOW_THRESHOLD)>> {
};

template <typename U1, typename U2>
struct SameDimension : stdx::bool_constant<U1::dim_ == U2::dim_> {};

template <typename Rep, typename ScaleFactor, typename SourceRep>
struct CoreImplicitConversionPolicyImpl
    : stdx::disjunction<
          std::is_floating_point<Rep>,
          stdx::conjunction<std::is_integral<SourceRep>,
                            IsInteger<ScaleFactor>,
                            detail::CanScaleThresholdWithoutOverflow<Rep, ScaleFactor>>> {};

// Always permit the identity scaling.
template <typename Rep>
struct CoreImplicitConversionPolicyImpl<Rep, Magnitude<>, Rep> : std::true_type {};

template <typename Rep, typename ScaleFactor, typename SourceRep>
using CoreImplicitConversionPolicy = CoreImplicitConversionPolicyImpl<Rep, ScaleFactor, SourceRep>;

template <typename Rep, typename ScaleFactor, typename SourceRep>
struct PermitAsCarveOutForIntegerPromotion
    : stdx::conjunction<std::is_same<ScaleFactor, Magnitude<>>,
                        std::is_integral<Rep>,
                        std::is_integral<SourceRep>,
                        std::is_assignable<Rep &, SourceRep>> {};
}  // namespace detail

template <typename Rep, typename ScaleFactor>
struct ImplicitRepPermitted : detail::CoreImplicitConversionPolicy<Rep, ScaleFactor, Rep> {};

template <typename Rep, typename SourceUnitSlot, typename TargetUnitSlot>
constexpr bool implicit_rep_permitted_from_source_to_target(SourceUnitSlot, TargetUnitSlot) {
    using SourceUnit = AssociatedUnitT<SourceUnitSlot>;
    using TargetUnit = AssociatedUnitT<TargetUnitSlot>;
    static_assert(HasSameDimension<SourceUnit, TargetUnit>::value,
                  "Can only convert same-dimension units");

    return ImplicitRepPermitted<Rep, UnitRatioT<SourceUnit, TargetUnit>>::value;
}

template <typename Unit, typename Rep>
struct ConstructionPolicy {
    // Note: it's tempting to use the UnitRatioT trait here, but we can't, because it produces a
    // hard error for units with different dimensions.  This is for good reason: magnitude ratios
    // are meaningless unless the dimension is the same.  UnitRatioT is the user-facing tool, so we
    // build in this hard error for safety.  Here, we need a soft error, so we do the dimension
    // check manually below.
    template <typename SourceUnit>
    using ScaleFactor = MagQuotientT<detail::MagT<SourceUnit>, detail::MagT<Unit>>;

    template <typename SourceUnit, typename SourceRep>
    using PermitImplicitFrom = stdx::conjunction<
        HasSameDimension<Unit, SourceUnit>,
        stdx::disjunction<
            detail::CoreImplicitConversionPolicy<Rep, ScaleFactor<SourceUnit>, SourceRep>,
            detail::PermitAsCarveOutForIntegerPromotion<Rep, ScaleFactor<SourceUnit>, SourceRep>>>;
};

}  // namespace au


namespace au {
namespace detail {

// The various categories by which a magnitude can be applied to a numeric quantity.
enum class ApplyAs {
    INTEGER_MULTIPLY,
    INTEGER_DIVIDE,
    RATIONAL_MULTIPLY,
    IRRATIONAL_MULTIPLY,
};

template <typename... BPs>
constexpr ApplyAs categorize_magnitude(Magnitude<BPs...>) {
    if (IsInteger<Magnitude<BPs...>>::value) {
        return ApplyAs::INTEGER_MULTIPLY;
    }

    if (IsInteger<MagInverseT<Magnitude<BPs...>>>::value) {
        return ApplyAs::INTEGER_DIVIDE;
    }

    return IsRational<Magnitude<BPs...>>::value ? ApplyAs::RATIONAL_MULTIPLY
                                                : ApplyAs::IRRATIONAL_MULTIPLY;
}

template <typename Mag, ApplyAs Category, typename T, bool is_T_integral>
struct ApplyMagnitudeImpl;

template <typename T, bool IsMagnitudeValid>
struct OverflowChecker {
    // Default case: `IsMagnitudeValid` is true.
    static constexpr bool would_product_overflow(T x, T mag_value) {
        return (x > (std::numeric_limits<T>::max() / mag_value)) ||
               (x < (std::numeric_limits<T>::lowest() / mag_value));
    }
};

template <typename T>
struct OverflowChecker<T, false> {
    // Specialization for when `IsMagnitudeValid` is false.
    //
    // This means that the magnitude itself could not fit inside of the type; therefore, the only
    // possible value that would not overflow is zero.
    static constexpr bool would_product_overflow(T x, T) { return (x != T{0}); }
};

template <typename T, bool IsTIntegral>
struct TruncationCheckerIfMagnitudeValid {
    // Default case: T is integral.
    static_assert(std::is_integral<T>::value && IsTIntegral,
                  "Mismatched instantiation (should never be done manually)");

    static constexpr bool would_truncate(T x, T mag_value) { return (x % mag_value != T{0}); }
};

template <typename T>
struct TruncationCheckerIfMagnitudeValid<T, false> {
    // Specialization for when T is not integral: by convention, assume no truncation for floats.
    static_assert(!std::is_integral<T>::value,
                  "Mismatched instantiation (should never be done manually)");
    static constexpr bool would_truncate(T, T) { return false; }
};

template <typename T, bool IsMagnitudeValid>
// Default case: `IsMagnitudeValid` is true.
struct TruncationChecker : TruncationCheckerIfMagnitudeValid<T, std::is_integral<T>::value> {
    static_assert(IsMagnitudeValid, "Mismatched instantiation (should never be done manually)");
};

template <typename T>
struct TruncationChecker<T, false> {
    // Specialization for when `IsMagnitudeValid` is false.
    //
    // This means that the magnitude itself could not fit inside of the type; therefore, the only
    // possible value that would not truncate is zero.
    static constexpr bool would_truncate(T x, T) { return (x != T{0}); }
};

// Multiplying by an integer, for any type T.
template <typename Mag, typename T, bool is_T_integral>
struct ApplyMagnitudeImpl<Mag, ApplyAs::INTEGER_MULTIPLY, T, is_T_integral> {
    static_assert(categorize_magnitude(Mag{}) == ApplyAs::INTEGER_MULTIPLY,
                  "Mismatched instantiation (should never be done manually)");
    static_assert(is_T_integral == std::is_integral<T>::value,
                  "Mismatched instantiation (should never be done manually)");

    constexpr T operator()(const T &x) { return x * get_value<T>(Mag{}); }

    static constexpr bool would_overflow(const T &x) {
        constexpr auto mag_value_result = get_value_result<T>(Mag{});
        return OverflowChecker<T, mag_value_result.outcome == MagRepresentationOutcome::OK>::
            would_product_overflow(x, mag_value_result.value);
    }

    static constexpr bool would_truncate(const T &) { return false; }
};

// Dividing by an integer, for any type T.
template <typename Mag, typename T, bool is_T_integral>
struct ApplyMagnitudeImpl<Mag, ApplyAs::INTEGER_DIVIDE, T, is_T_integral> {
    static_assert(categorize_magnitude(Mag{}) == ApplyAs::INTEGER_DIVIDE,
                  "Mismatched instantiation (should never be done manually)");
    static_assert(is_T_integral == std::is_integral<T>::value,
                  "Mismatched instantiation (should never be done manually)");

    constexpr T operator()(const T &x) { return x / get_value<T>(MagInverseT<Mag>{}); }

    static constexpr bool would_overflow(const T &) { return false; }

    static constexpr bool would_truncate(const T &x) {
        constexpr auto mag_value_result = get_value_result<T>(MagInverseT<Mag>{});
        return TruncationChecker<T, mag_value_result.outcome == MagRepresentationOutcome::OK>::
            would_truncate(x, mag_value_result.value);
    }
};

template <typename T, typename Mag, bool is_T_signed>
struct RationalOverflowChecker;
template <typename T, typename Mag>
struct RationalOverflowChecker<T, Mag, true> {
    static constexpr bool would_overflow(const T &x) {
        static_assert(std::is_signed<T>::value,
                      "Mismatched instantiation (should never be done manually)");
        const bool safe = (x <= MaxNonOverflowingValue<T, Mag>::value()) &&
                          (x >= MinNonOverflowingValue<T, Mag>::value());
        return !safe;
    }
};
template <typename T, typename Mag>
struct RationalOverflowChecker<T, Mag, false> {
    static constexpr bool would_overflow(const T &x) {
        static_assert(!std::is_signed<T>::value,
                      "Mismatched instantiation (should never be done manually)");
        const bool safe = (x <= MaxNonOverflowingValue<T, Mag>::value());
        return !safe;
    }
};

// Applying a (non-integer, non-inverse-integer) rational, for any integral type T.
template <typename Mag, typename T>
struct ApplyMagnitudeImpl<Mag, ApplyAs::RATIONAL_MULTIPLY, T, true> {
    static_assert(categorize_magnitude(Mag{}) == ApplyAs::RATIONAL_MULTIPLY,
                  "Mismatched instantiation (should never be done manually)");
    static_assert(std::is_integral<T>::value,
                  "Mismatched instantiation (should never be done manually)");

    constexpr T operator()(const T &x) {
        using P = PromotedType<T>;
        return static_cast<T>(x * get_value<P>(numerator(Mag{})) /
                              get_value<P>(denominator(Mag{})));
    }

    static constexpr bool would_overflow(const T &x) {
        return RationalOverflowChecker<T, Mag, std::is_signed<T>::value>::would_overflow(x);
    }

    static constexpr bool would_truncate(const T &x) {
        constexpr auto mag_value_result = get_value_result<T>(denominator(Mag{}));
        return TruncationChecker<T, mag_value_result.outcome == MagRepresentationOutcome::OK>::
            would_truncate(x, mag_value_result.value);
    }
};

// Applying a (non-integer, non-inverse-integer) rational, for any non-integral type T.
template <typename Mag, typename T>
struct ApplyMagnitudeImpl<Mag, ApplyAs::RATIONAL_MULTIPLY, T, false> {
    static_assert(categorize_magnitude(Mag{}) == ApplyAs::RATIONAL_MULTIPLY,
                  "Mismatched instantiation (should never be done manually)");
    static_assert(!std::is_integral<T>::value,
                  "Mismatched instantiation (should never be done manually)");

    constexpr T operator()(const T &x) { return x * get_value<T>(Mag{}); }

    static constexpr bool would_overflow(const T &x) {
        constexpr auto mag_value_result = get_value_result<T>(Mag{});
        return OverflowChecker<T, mag_value_result.outcome == MagRepresentationOutcome::OK>::
            would_product_overflow(x, mag_value_result.value);
    }

    static constexpr bool would_truncate(const T &) { return false; }
};

// Applying an irrational for any type T (although only non-integral T makes sense).
template <typename Mag, typename T, bool is_T_integral>
struct ApplyMagnitudeImpl<Mag, ApplyAs::IRRATIONAL_MULTIPLY, T, is_T_integral> {
    static_assert(!std::is_integral<T>::value, "Cannot apply irrational magnitude to integer type");

    static_assert(categorize_magnitude(Mag{}) == ApplyAs::IRRATIONAL_MULTIPLY,
                  "Mismatched instantiation (should never be done manually)");
    static_assert(is_T_integral == std::is_integral<T>::value,
                  "Mismatched instantiation (should never be done manually)");

    constexpr T operator()(const T &x) { return x * get_value<T>(Mag{}); }

    static constexpr bool would_overflow(const T &x) {
        constexpr auto mag_value_result = get_value_result<T>(Mag{});
        return OverflowChecker<T, mag_value_result.outcome == MagRepresentationOutcome::OK>::
            would_product_overflow(x, mag_value_result.value);
    }

    static constexpr bool would_truncate(const T &) { return false; }
};

template <typename T, typename MagT>
struct ApplyMagnitudeType;
template <typename T, typename MagT>
using ApplyMagnitudeT = typename ApplyMagnitudeType<T, MagT>::type;
template <typename T, typename... BPs>
struct ApplyMagnitudeType<T, Magnitude<BPs...>>
    : stdx::type_identity<ApplyMagnitudeImpl<Magnitude<BPs...>,
                                             categorize_magnitude(Magnitude<BPs...>{}),
                                             T,
                                             std::is_integral<T>::value>> {};

template <typename T, typename... BPs>
constexpr T apply_magnitude(const T &x, Magnitude<BPs...>) {
    return ApplyMagnitudeT<T, Magnitude<BPs...>>{}(x);
}

}  // namespace detail
}  // namespace au



namespace au {

template <typename UnitT>
struct QuantityMaker;

template <typename UnitT, typename RepT>
class Quantity;

//
// Make a Quantity of the given Unit, which has this value as measured in the Unit.
//
template <typename UnitT, typename T>
constexpr auto make_quantity(T value) {
    return QuantityMaker<UnitT>{}(value);
}

template <typename Unit, typename T>
constexpr auto make_quantity_unless_unitless(T value) {
    return std::conditional_t<IsUnitlessUnit<Unit>::value, stdx::identity, QuantityMaker<Unit>>{}(
        value);
}

// Trait to check whether two Quantity types are exactly equivalent.
//
// For purposes of our library, "equivalent" means that they have the same Dimension and Magnitude.
template <typename Q1, typename Q2>
struct AreQuantityTypesEquivalent;

// Trait for a type T which corresponds exactly to some Quantity type.
//
// "Correspondence" with a `Quantity<U, R>` means that T stores a value in a numeric datatype R, and
// this value represents a quantity whose unit of measure is quantity-equivalent to U.
//
// The canonical examples are the `duration` types from the `std::chrono::library`.  For example,
// `std::chrono::duration<double, std::nano>` exactly corresponds to `QuantityD<Nano<Seconds>>`, and
// it is always OK to convert back and forth between these types implicitly.
//
// To add support for a type T which is equivalent to Quantity<U, R>, define a specialization of
// `CorrespondingQuantity<T>` with a member alias `Unit` for `U`, and `Rep` for `R`.  You should
// then add static member functions as follows to add support for each direction of conversion.
//   - For T -> Quantity, define `R extract_value(T)`.
//   - For Quantity -> T, define `T construct_from_value(R)`.
template <typename T>
struct CorrespondingQuantity {};
template <typename T>
using CorrespondingQuantityT =
    Quantity<typename CorrespondingQuantity<T>::Unit, typename CorrespondingQuantity<T>::Rep>;

// Redirect various cvref-qualified specializations to the "main" specialization.
//
// We use this slightly counterintuitive approach, rather than a more conventional
// `remove_cvref_t`-based approach, because the latter causes an _internal compiler error_ on the
// ACI QNX build.
template <typename T>
struct CorrespondingQuantity<const T> : CorrespondingQuantity<T> {};
template <typename T>
struct CorrespondingQuantity<T &> : CorrespondingQuantity<T> {};
template <typename T>
struct CorrespondingQuantity<const T &> : CorrespondingQuantity<T> {};

// Request conversion of any type to its corresponding Quantity, if there is one.
//
// This is a way to explicitly and readably "enter the au Quantity domain" when we have some
// non-au-Quantity type which is nevertheless exactly and unambiguously equivalent to some Quantity.
//
// `as_quantity()` is SFINAE-friendly: we can use it to constrain templates to types `T` which are
// exactly equivalent to some Quantity type.
template <typename T>
constexpr auto as_quantity(T &&x) -> CorrespondingQuantityT<T> {
    using Q = CorrespondingQuantity<T>;
    static_assert(IsUnit<typename Q::Unit>{}, "No Quantity corresponding to type");

    auto value = Q::extract_value(std::forward<T>(x));
    static_assert(std::is_same<decltype(value), typename Q::Rep>{},
                  "Inconsistent CorrespondingQuantity implementation");

    return make_quantity<typename Q::Unit>(value);
}

template <typename UnitT, typename RepT>
class Quantity {
    template <bool ImplicitOk, typename OtherUnit, typename OtherRep>
    using EnableIfImplicitOkIs = std::enable_if_t<
        ImplicitOk ==
        ConstructionPolicy<UnitT, RepT>::template PermitImplicitFrom<OtherUnit, OtherRep>::value>;

 public:
    using Rep = RepT;
    using Unit = UnitT;
    static constexpr auto unit = Unit{};

    // IMPLICIT constructor for another Quantity of the same Dimension.
    template <typename OtherUnit,
              typename OtherRep,
              typename Enable = EnableIfImplicitOkIs<true, OtherUnit, OtherRep>>
    constexpr Quantity(Quantity<OtherUnit, OtherRep> other)  // NOLINT(runtime/explicit)
        : Quantity{other.template as<Rep>(UnitT{})} {}

    // EXPLICIT constructor for another Quantity of the same Dimension.
    template <typename OtherUnit,
              typename OtherRep,
              typename Enable = EnableIfImplicitOkIs<false, OtherUnit, OtherRep>,
              typename ThisUnusedTemplateParameterDistinguishesUsFromTheAboveConstructor = void>
    // Deleted: use `.as<NewRep>(new_unit)` to force a cast.
    explicit constexpr Quantity(Quantity<OtherUnit, OtherRep> other) = delete;

    // Construct this Quantity with a value of exactly Zero.
    constexpr Quantity(Zero) : value_{0} {}

    constexpr Quantity() noexcept = default;

    // Implicit construction from any exactly-equivalent type.
    template <
        typename T,
        std::enable_if_t<std::is_convertible<CorrespondingQuantityT<T>, Quantity>::value, int> = 0>
    constexpr Quantity(T &&x) : Quantity{as_quantity(std::forward<T>(x))} {}

    template <typename NewRep,
              typename NewUnit,
              typename = std::enable_if_t<IsUnit<AssociatedUnitT<NewUnit>>::value>>
    constexpr auto as(NewUnit) const {
        using Common = std::common_type_t<Rep, NewRep>;
        using Factor = UnitRatioT<AssociatedUnitT<Unit>, AssociatedUnitT<NewUnit>>;

        return make_quantity<AssociatedUnitT<NewUnit>>(
            static_cast<NewRep>(detail::apply_magnitude(static_cast<Common>(value_), Factor{})));
    }

    template <typename NewUnit,
              typename = std::enable_if_t<IsUnit<AssociatedUnitT<NewUnit>>::value>>
    constexpr auto as(NewUnit u) const {
        constexpr bool IMPLICIT_OK =
            implicit_rep_permitted_from_source_to_target<Rep>(unit, NewUnit{});
        constexpr bool INTEGRAL_REP = std::is_integral<Rep>::value;
        static_assert(
            IMPLICIT_OK || INTEGRAL_REP,
            "Should never occur.  In the following static_assert, we assume that IMPLICIT_OK "
            "can never fail unless INTEGRAL_REP is true.");
        static_assert(
            IMPLICIT_OK,
            "Dangerous conversion for integer Rep!  See: "
            "https://aurora-opensource.github.io/au/main/troubleshooting/#dangerous-conversion");
        return as<Rep>(u);
    }

    template <typename NewRep,
              typename NewUnit,
              typename = std::enable_if_t<IsUnit<AssociatedUnitT<NewUnit>>::value>>
    constexpr NewRep in(NewUnit u) const {
        if (are_units_quantity_equivalent(unit, u) && std::is_same<Rep, NewRep>::value) {
            return static_cast<NewRep>(value_);
        } else {
            return as<NewRep>(u).in(u);
        }
    }

    template <typename NewUnit,
              typename = std::enable_if_t<IsUnit<AssociatedUnitT<NewUnit>>::value>>
    constexpr Rep in(NewUnit u) const {
        if (are_units_quantity_equivalent(unit, u)) {
            return value_;
        } else {
            // Since Rep was requested _implicitly_, delegate to `.as()` for its safety checks.
            return as(u).in(u);
        }
    }

    // "Old-style" overloads with <U, R> template parameters, and no function parameters.
    //
    // Matches the syntax from the CppCon 2021 talk, and legacy Aurora usage.
    template <typename U>
    [[deprecated(
        "Do not write `.as<YourUnits>()`; write `.as(your_units)` instead.")]] constexpr auto
    as() const -> decltype(as(U{})) {
        return as(U{});
    }
    template <typename U, typename R, typename = std::enable_if_t<IsUnit<U>::value>>
    [[deprecated(
        "Do not write `.as<YourUnits, T>()`; write `.as<T>(your_units)` instead.")]] constexpr auto
    as() const {
        return as<R>(U{});
    }
    template <typename U>
    [[deprecated(
        "Do not write `.in<YourUnits>()`; write `.in(your_units)` instead.")]] constexpr auto
    in() const -> decltype(in(U{})) {
        return in(U{});
    }
    template <typename U, typename R, typename = std::enable_if_t<IsUnit<U>::value>>
    [[deprecated(
        "Do not write `.in<YourUnits, T>()`; write `.in<T>(your_units)` instead.")]] constexpr auto
    in() const {
        return in<R>(U{});
    }

    // "Forcing" conversions, which explicitly ignore safety checks for overflow and truncation.
    template <typename NewUnit>
    constexpr auto coerce_as(NewUnit) const {
        // Usage example: `q.coerce_as(new_units)`.
        return as<Rep>(NewUnit{});
    }
    template <typename NewRep, typename NewUnit>
    constexpr auto coerce_as(NewUnit) const {
        // Usage example: `q.coerce_as<T>(new_units)`.
        return as<NewRep>(NewUnit{});
    }
    template <typename NewUnit>
    constexpr auto coerce_in(NewUnit) const {
        // Usage example: `q.coerce_in(new_units)`.
        return in<Rep>(NewUnit{});
    }
    template <typename NewRep, typename NewUnit>
    constexpr auto coerce_in(NewUnit) const {
        // Usage example: `q.coerce_in<T>(new_units)`.
        return in<NewRep>(NewUnit{});
    }

    // Direct access to the underlying value member, with any Quantity-equivalent Unit.
    //
    // Mutable access, QuantityMaker input.
    template <typename U>
    Rep &data_in(const QuantityMaker<U> &) {
        static_assert(AreUnitsQuantityEquivalent<U, Unit>::value,
                      "Can only access value via Quantity-equivalent unit");
        return value_;
    }
    // Mutable access, Unit input.
    template <typename U>
    Rep &data_in(const U &) {
        return data_in(QuantityMaker<U>{});
    }
    // Const access, QuantityMaker input.
    template <typename U>
    const Rep &data_in(const QuantityMaker<U> &) const {
        static_assert(AreUnitsQuantityEquivalent<U, Unit>::value,
                      "Can only access value via Quantity-equivalent unit");
        return value_;
    }
    // Const access, Unit input.
    template <typename U>
    const Rep &data_in(const U &) const {
        return data_in(QuantityMaker<U>{});
    }

    // Permit this factory functor to access our private constructor.
    //
    // We allow this because it explicitly names the unit at the callsite, even if people refer to
    // this present Quantity type by an alias that omits the unit.  This preserves Unit Safety and
    // promotes callsite readability.
    friend struct QuantityMaker<UnitT>;

    // Comparison operators.
    friend constexpr bool operator==(Quantity a, Quantity b) { return a.value_ == b.value_; }
    friend constexpr bool operator!=(Quantity a, Quantity b) { return a.value_ != b.value_; }
    friend constexpr bool operator<(Quantity a, Quantity b) { return a.value_ < b.value_; }
    friend constexpr bool operator<=(Quantity a, Quantity b) { return a.value_ <= b.value_; }
    friend constexpr bool operator>(Quantity a, Quantity b) { return a.value_ > b.value_; }
    friend constexpr bool operator>=(Quantity a, Quantity b) { return a.value_ >= b.value_; }

    // Addition and subtraction for like quantities.
    friend constexpr Quantity<UnitT, decltype(std::declval<RepT>() + std::declval<RepT>())>
    operator+(Quantity a, Quantity b) {
        return make_quantity<UnitT>(a.value_ + b.value_);
    }
    friend constexpr Quantity<UnitT, decltype(std::declval<RepT>() - std::declval<RepT>())>
    operator-(Quantity a, Quantity b) {
        return make_quantity<UnitT>(a.value_ - b.value_);
    }

    // Scalar multiplication.
    template <typename T, typename = std::enable_if_t<std::is_arithmetic<T>::value>>
    friend constexpr auto operator*(Quantity a, T s) {
        return make_quantity<UnitT>(a.value_ * s);
    }
    template <typename T, typename = std::enable_if_t<std::is_arithmetic<T>::value>>
    friend constexpr auto operator*(T s, Quantity a) {
        return make_quantity<UnitT>(s * a.value_);
    }

    // Scalar division.
    template <typename T, typename = std::enable_if_t<std::is_arithmetic<T>::value>>
    friend constexpr auto operator/(Quantity a, T s) {
        return make_quantity<UnitT>(a.value_ / s);
    }
    template <typename T, typename = std::enable_if_t<std::is_arithmetic<T>::value>>
    friend constexpr auto operator/(T s, Quantity a) {
        warn_if_integer_division<T>();
        return make_quantity<decltype(pow<-1>(unit))>(s / a.value_);
    }

    // Multiplication for dimensioned quantities.
    template <typename OtherUnit, typename OtherRep>
    constexpr auto operator*(Quantity<OtherUnit, OtherRep> q) const {
        return make_quantity_unless_unitless<UnitProductT<Unit, OtherUnit>>(value_ *
                                                                            q.in(OtherUnit{}));
    }

    // Division for dimensioned quantities.
    template <typename OtherUnit, typename OtherRep>
    constexpr auto operator/(Quantity<OtherUnit, OtherRep> q) const {
        warn_if_integer_division<OtherRep>();
        return make_quantity_unless_unitless<UnitQuotientT<Unit, OtherUnit>>(value_ /
                                                                             q.in(OtherUnit{}));
    }

    // Short-hand addition and subtraction assignment.
    constexpr Quantity &operator+=(Quantity other) {
        value_ += other.value_;
        return *this;
    }
    constexpr Quantity &operator-=(Quantity other) {
        value_ -= other.value_;
        return *this;
    }

    // Short-hand multiplication assignment.
    template <typename T>
    constexpr Quantity &operator*=(T s) {
        static_assert(
            std::is_arithmetic<T>::value,
            "This overload is only for scalar multiplication-assignment with arithmetic types");

        static_assert(
            std::is_floating_point<Rep>::value || std::is_integral<T>::value,
            "We don't support compound multiplication of integral types by floating point");

        value_ *= s;
        return *this;
    }

    // Short-hand division assignment.
    template <typename T>
    constexpr Quantity &operator/=(T s) {
        static_assert(std::is_arithmetic<T>::value,
                      "This overload is only for scalar division-assignment with arithmetic types");

        static_assert(std::is_floating_point<Rep>::value || std::is_integral<T>::value,
                      "We don't support compound division of integral types by floating point");

        value_ /= s;
        return *this;
    }

    // Unary plus and minus.
    constexpr Quantity operator+() const { return {+value_}; }
    constexpr Quantity operator-() const { return {-value_}; }

    // Automatic conversion to Rep for Unitless type.
    template <typename U = UnitT, typename = std::enable_if_t<IsUnitlessUnit<U>::value>>
    constexpr operator Rep() const {
        return value_;
    }

    // Automatic conversion to any equivalent type that supports it.
    template <
        typename T,
        std::enable_if_t<std::is_convertible<Quantity, CorrespondingQuantityT<T>>::value, int> = 0>
    constexpr operator T() const {
        return CorrespondingQuantity<T>::construct_from_value(
            CorrespondingQuantityT<T>{*this}.in(typename CorrespondingQuantity<T>::Unit{}));
    }

 private:
    template <typename OtherRep>
    static constexpr void warn_if_integer_division() {
        constexpr bool uses_integer_division =
            (std::is_integral<Rep>::value && std::is_integral<OtherRep>::value);
        static_assert(!uses_integer_division,
                      "Integer division forbidden: use integer_quotient() if you really want it");
    }

    constexpr Quantity(Rep value) : value_{value} {}

    Rep value_{0};
};

// Force integer division beteween two integer Quantities, in a callsite-obvious way.
template <typename U1, typename R1, typename U2, typename R2>
constexpr auto integer_quotient(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    static_assert(std::is_integral<R1>::value && std::is_integral<R2>::value,
                  "integer_quotient() can only be called with integral Rep");
    return make_quantity<UnitQuotientT<U1, U2>>(q1.in(U1{}) / q2.in(U2{}));
}

// Force integer division beteween an integer Quantity and a raw number.
template <typename U, typename R, typename T>
constexpr auto integer_quotient(Quantity<U, R> q, T x) {
    static_assert(std::is_integral<R>::value && std::is_integral<T>::value,
                  "integer_quotient() can only be called with integral Rep");
    return make_quantity<U>(q.in(U{}) / x);
}

// Force integer division beteween a raw number and an integer Quantity.
template <typename T, typename U, typename R>
constexpr auto integer_quotient(T x, Quantity<U, R> q) {
    static_assert(std::is_integral<T>::value && std::is_integral<R>::value,
                  "integer_quotient() can only be called with integral Rep");
    return make_quantity<UnitInverseT<U>>(x / q.in(U{}));
}

// The modulo operator (i.e., the remainder of an integer division).
//
// Only defined whenever (R1{} % R2{}) is defined (i.e., for integral Reps), _and_
// `CommonUnitT<U1, U2>` is also defined.  We convert to that common unit to perform the operation.
template <typename U1, typename R1, typename U2, typename R2>
constexpr auto operator%(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    using U = CommonUnitT<U1, U2>;
    return make_quantity<U>(q1.in(U{}) % q2.in(U{}));
}

// Type trait to detect whether two Quantity types are equivalent.
//
// In this library, Quantity types are "equivalent" exactly when they use the same Rep, and are
// based on equivalent units.
template <typename U1, typename U2, typename R1, typename R2>
struct AreQuantityTypesEquivalent<Quantity<U1, R1>, Quantity<U2, R2>>
    : stdx::conjunction<std::is_same<R1, R2>, AreUnitsQuantityEquivalent<U1, U2>> {};

// Cast Quantity to a different underlying type.
template <typename NewRep, typename Unit, typename Rep>
constexpr auto rep_cast(Quantity<Unit, Rep> q) {
    return q.template as<NewRep>(Unit{});
}

// Help Zero act more faithfully like a Quantity.
//
// Casting Zero to any "Rep" is trivial, because it has no Rep, and is already consistent with all.
template <typename NewRep>
constexpr auto rep_cast(Zero z) {
    return z;
}

//
// Quantity aliases to set a particular Rep.
//
// This presents a less cumbersome interface for end users.
//
template <typename UnitT>
using QuantityD = Quantity<UnitT, double>;
template <typename UnitT>
using QuantityF = Quantity<UnitT, float>;
template <typename UnitT>
using QuantityI = Quantity<UnitT, int>;
template <typename UnitT>
using QuantityU = Quantity<UnitT, unsigned int>;
template <typename UnitT>
using QuantityI32 = Quantity<UnitT, int32_t>;
template <typename UnitT>
using QuantityU32 = Quantity<UnitT, uint32_t>;
template <typename UnitT>
using QuantityI64 = Quantity<UnitT, int64_t>;
template <typename UnitT>
using QuantityU64 = Quantity<UnitT, uint64_t>;

template <typename UnitT>
struct QuantityMaker {
    using Unit = UnitT;
    static constexpr auto unit = Unit{};

    template <typename T>
    constexpr Quantity<Unit, T> operator()(T value) const {
        return {value};
    }

    template <typename... BPs>
    constexpr auto operator*(Magnitude<BPs...> m) const {
        return QuantityMaker<decltype(unit * m)>{};
    }

    template <typename... BPs>
    constexpr auto operator/(Magnitude<BPs...> m) const {
        return QuantityMaker<decltype(unit / m)>{};
    }

    template <typename DivisorUnit>
    constexpr auto operator/(SingularNameFor<DivisorUnit>) const {
        return QuantityMaker<UnitQuotientT<Unit, DivisorUnit>>{};
    }

    template <typename MultiplierUnit>
    friend constexpr auto operator*(SingularNameFor<MultiplierUnit>, QuantityMaker) {
        return QuantityMaker<UnitProductT<MultiplierUnit, Unit>>{};
    }

    template <typename OtherUnit>
    constexpr auto operator*(QuantityMaker<OtherUnit>) const {
        return QuantityMaker<UnitProductT<Unit, OtherUnit>>{};
    }

    template <typename OtherUnit>
    constexpr auto operator/(QuantityMaker<OtherUnit>) const {
        return QuantityMaker<UnitQuotientT<Unit, OtherUnit>>{};
    }
};

template <typename U>
struct AssociatedUnit<QuantityMaker<U>> : stdx::type_identity<U> {};

template <int Exp, typename Unit>
constexpr auto pow(QuantityMaker<Unit>) {
    return QuantityMaker<UnitPowerT<Unit, Exp>>{};
}

template <int N, typename Unit>
constexpr auto root(QuantityMaker<Unit>) {
    return QuantityMaker<UnitPowerT<Unit, 1, N>>{};
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Runtime conversion checkers

// Check conversion for overflow (no change of rep).
template <typename U, typename R, typename TargetUnitSlot>
constexpr bool will_conversion_overflow(Quantity<U, R> q, TargetUnitSlot target_unit) {
    return detail::ApplyMagnitudeT<R, decltype(unit_ratio(U{}, target_unit))>::would_overflow(
        q.in(U{}));
}

// Check conversion for truncation (no change of rep).
template <typename U, typename R, typename TargetUnitSlot>
constexpr bool will_conversion_truncate(Quantity<U, R> q, TargetUnitSlot target_unit) {
    return detail::ApplyMagnitudeT<R, decltype(unit_ratio(U{}, target_unit))>::would_truncate(
        q.in(U{}));
}

// Check for any lossiness in conversion (no change of rep).
template <typename U, typename R, typename TargetUnitSlot>
constexpr bool is_conversion_lossy(Quantity<U, R> q, TargetUnitSlot target_unit) {
    return will_conversion_truncate(q, target_unit) || will_conversion_overflow(q, target_unit);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Comparing and/or combining Quantities of different types.

namespace detail {
// Helper to cast this Quantity to its common type with some other Quantity (explicitly supplied).
//
// Note that `TargetUnit` is supposed to be the common type of the input Quantity and some other
// Quantity.  This function should never be called directly; it should only be called by
// `using_common_type()`.  The program behaviour is undefined if anyone calls this function
// directly.  (In particular, we explicitly assume that the conversion to the Rep of TargetUnit is
// not narrowing for the input Quantity.)
//
// We would have liked this to just be a simple lambda, but some old compilers sometimes struggle
// with understanding that the lambda implementation of this can be constexpr.
template <typename TargetUnit, typename U, typename R>
constexpr auto cast_to_common_type(Quantity<U, R> q) {
    // When we perform a unit conversion to U, we need to make sure the library permits this
    // conversion *implicitly* for a rep R.  The form `rep_cast<R>(q).as(U{})` achieves
    // this.  First, we cast the Rep to R (which will typically be the wider of the input Reps).
    // Then, we use the *unit-only* form of the conversion operator: `as(U{})`, not
    // `as<R>(U{})`, because only the former actually checks the conversion policy.
    return rep_cast<typename TargetUnit::Rep>(q).as(TargetUnit::unit);
}

template <typename T, typename U, typename Func>
constexpr auto using_common_type(T t, U u, Func f) {
    using C = std::common_type_t<T, U>;
    static_assert(
        std::is_same<typename C::Rep, std::common_type_t<typename T::Rep, typename U::Rep>>::value,
        "Rep of common type is not common type of Reps (this should never occur)");

    return f(cast_to_common_type<C>(t), cast_to_common_type<C>(u));
}
}  // namespace detail

// Comparison functions for compatible Quantity types.
template <typename U1, typename U2, typename R1, typename R2>
constexpr bool operator==(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    return detail::using_common_type(q1, q2, detail::equal);
}
template <typename U1, typename U2, typename R1, typename R2>
constexpr bool operator!=(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    return detail::using_common_type(q1, q2, detail::not_equal);
}
template <typename U1, typename U2, typename R1, typename R2>
constexpr bool operator<(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    return detail::using_common_type(q1, q2, detail::less);
}
template <typename U1, typename U2, typename R1, typename R2>
constexpr bool operator<=(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    return detail::using_common_type(q1, q2, detail::less_equal);
}
template <typename U1, typename U2, typename R1, typename R2>
constexpr bool operator>(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    return detail::using_common_type(q1, q2, detail::greater);
}
template <typename U1, typename U2, typename R1, typename R2>
constexpr bool operator>=(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    return detail::using_common_type(q1, q2, detail::greater_equal);
}

// Addition and subtraction functions for compatible Quantity types.
template <typename U1, typename U2, typename R1, typename R2>
constexpr auto operator+(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    return detail::using_common_type(q1, q2, detail::plus);
}
template <typename U1, typename U2, typename R1, typename R2>
constexpr auto operator-(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    return detail::using_common_type(q1, q2, detail::minus);
}

// Mixed-type operations with a left-Quantity, and right-Quantity-equivalent.
template <typename U, typename R, typename QLike>
constexpr auto operator+(Quantity<U, R> q1, QLike q2) -> decltype(q1 + as_quantity(q2)) {
    return q1 + as_quantity(q2);
}
template <typename U, typename R, typename QLike>
constexpr auto operator-(Quantity<U, R> q1, QLike q2) -> decltype(q1 - as_quantity(q2)) {
    return q1 - as_quantity(q2);
}
template <typename U, typename R, typename QLike>
constexpr auto operator==(Quantity<U, R> q1, QLike q2) -> decltype(q1 == as_quantity(q2)) {
    return q1 == as_quantity(q2);
}
template <typename U, typename R, typename QLike>
constexpr auto operator!=(Quantity<U, R> q1, QLike q2) -> decltype(q1 != as_quantity(q2)) {
    return q1 != as_quantity(q2);
}
template <typename U, typename R, typename QLike>
constexpr auto operator<(Quantity<U, R> q1, QLike q2) -> decltype(q1 < as_quantity(q2)) {
    return q1 < as_quantity(q2);
}
template <typename U, typename R, typename QLike>
constexpr auto operator<=(Quantity<U, R> q1, QLike q2) -> decltype(q1 <= as_quantity(q2)) {
    return q1 <= as_quantity(q2);
}
template <typename U, typename R, typename QLike>
constexpr auto operator>(Quantity<U, R> q1, QLike q2) -> decltype(q1 > as_quantity(q2)) {
    return q1 > as_quantity(q2);
}
template <typename U, typename R, typename QLike>
constexpr auto operator>=(Quantity<U, R> q1, QLike q2) -> decltype(q1 >= as_quantity(q2)) {
    return q1 >= as_quantity(q2);
}

// Mixed-type operations with a left-Quantity-equivalent, and right-Quantity.
template <typename U, typename R, typename QLike>
constexpr auto operator+(QLike q1, Quantity<U, R> q2) -> decltype(as_quantity(q1) + q2) {
    return as_quantity(q1) + q2;
}
template <typename U, typename R, typename QLike>
constexpr auto operator-(QLike q1, Quantity<U, R> q2) -> decltype(as_quantity(q1) - q2) {
    return as_quantity(q1) - q2;
}
template <typename U, typename R, typename QLike>
constexpr auto operator==(QLike q1, Quantity<U, R> q2) -> decltype(as_quantity(q1) == q2) {
    return as_quantity(q1) == q2;
}
template <typename U, typename R, typename QLike>
constexpr auto operator!=(QLike q1, Quantity<U, R> q2) -> decltype(as_quantity(q1) != q2) {
    return as_quantity(q1) != q2;
}
template <typename U, typename R, typename QLike>
constexpr auto operator<(QLike q1, Quantity<U, R> q2) -> decltype(as_quantity(q1) < q2) {
    return as_quantity(q1) < q2;
}
template <typename U, typename R, typename QLike>
constexpr auto operator<=(QLike q1, Quantity<U, R> q2) -> decltype(as_quantity(q1) <= q2) {
    return as_quantity(q1) <= q2;
}
template <typename U, typename R, typename QLike>
constexpr auto operator>(QLike q1, Quantity<U, R> q2) -> decltype(as_quantity(q1) > q2) {
    return as_quantity(q1) > q2;
}
template <typename U, typename R, typename QLike>
constexpr auto operator>=(QLike q1, Quantity<U, R> q2) -> decltype(as_quantity(q1) >= q2) {
    return as_quantity(q1) >= q2;
}

// Helper to compute the `std::common_type_t` of two `Quantity` types.
//
// `std::common_type` requires its specializations to be SFINAE-friendly, meaning that the `type`
// member should not exist for specializations with no common type.  Unfortunately, we can't
// directly use SFINAE on `std::common_type`.  What we can do is inherit our specialization's
// implementation from a different structure which we fully control, and which either has or doesn't
// have a `type` member as appropriate.
template <typename Q1, typename Q2, typename Enable = void>
struct CommonQuantity {};
template <typename U1, typename U2, typename R1, typename R2>
struct CommonQuantity<Quantity<U1, R1>,
                      Quantity<U2, R2>,
                      std::enable_if_t<HasSameDimension<U1, U2>::value>>
    : stdx::type_identity<Quantity<CommonUnitT<U1, U2>, std::common_type_t<R1, R2>>> {};
}  // namespace au

namespace std {
// Note: we would prefer not to reopen `namespace std` [1].  However, some older compilers (which we
// still want to support) incorrectly treat the preferred syntax recommended in [1] as an error.
// This usage does not encounter any of the pitfalls described in that link, so we use it.
//
// [1] https://quuxplusone.github.io/blog/2021/10/27/dont-reopen-namespace-std/
template <typename U1, typename U2, typename R1, typename R2>
struct common_type<au::Quantity<U1, R1>, au::Quantity<U2, R2>>
    : au::CommonQuantity<au::Quantity<U1, R1>, au::Quantity<U2, R2>> {};
}  // namespace std


namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/main/howto/new-units/).
template <typename T>
struct UnosLabel {
    static constexpr const char label[] = "U";
};
template <typename T>
constexpr const char UnosLabel<T>::label[];
struct Unos : UnitProductT<>, UnosLabel<void> {
    using UnosLabel<void>::label;
};
constexpr auto unos = QuantityMaker<Unos>{};

}  // namespace au


// "Mixin" classes to add operations for a "unit wrapper" --- that is, a template with a _single
// template parameter_ that is a unit.
//
// The operations are multiplication and division.  The mixins will specify what types the wrapper
// can combine with in this way, and what the resulting type will be.  They also take care of
// getting the resulting unit correct.  Finally, they handle integer division carefully.
//
// Every mixin has at least two template parameters.
//
//   1. The unit wrapper (a template template parameter).
//   2. The specific unit that it's wrapping (for convenience in the implementation).
//
// For mixins that compose with something that is _not_ a unit wrapper --- e.g., a raw number, or a
// magnitude --- this is all they need.  Other mixins compose with _other unit wrappers_, and these
// take two more template parameters: the wrapper we're composing with, and the resulting wrapper.

namespace au {
namespace detail {

// A SFINAE helper that is the identity, but only if we think a type is a valid rep.
//
// For now, we are restricting this to arithmetic types.  This doesn't mean they're the only reps we
// support; it just means they're the only reps we can _construct via this method_.  Later on, we
// would like to have a well-defined concept that defines what is and is not an acceptable rep for
// our `Quantity`.  Once we have that, we can simply constrain on that concept.  For more on this
// idea, see: https://github.com/aurora-opensource/au/issues/52
struct NoTypeMember {};
template <typename T>
struct TypeIdentityIfLooksLikeValidRep
    : std::conditional_t<std::is_arithmetic<T>::value, stdx::type_identity<T>, NoTypeMember> {};
template <typename T>
using TypeIdentityIfLooksLikeValidRepT = typename TypeIdentityIfLooksLikeValidRep<T>::type;

//
// A mixin that enables turning a raw number into a Quantity by multiplying or dividing.
//
template <template <typename U> class UnitWrapper, typename Unit>
struct MakesQuantityFromNumber {
    // (N * W), for number N and wrapper W.
    template <typename T>
    friend constexpr auto operator*(T x, UnitWrapper<Unit>)
        -> Quantity<Unit, TypeIdentityIfLooksLikeValidRepT<T>> {
        return make_quantity<Unit>(x);
    }

    // (W * N), for number N and wrapper W.
    template <typename T>
    friend constexpr auto operator*(UnitWrapper<Unit>, T x)
        -> Quantity<Unit, TypeIdentityIfLooksLikeValidRepT<T>> {
        return make_quantity<Unit>(x);
    }

    // (N / W), for number N and wrapper W.
    template <typename T>
    friend constexpr auto operator/(T x, UnitWrapper<Unit>)
        -> Quantity<UnitInverseT<Unit>, TypeIdentityIfLooksLikeValidRepT<T>> {
        return make_quantity<UnitInverseT<Unit>>(x);
    }

    // (W / N), for number N and wrapper W.
    template <typename T>
    friend constexpr auto operator/(UnitWrapper<Unit>, T x)
        -> Quantity<Unit, TypeIdentityIfLooksLikeValidRepT<T>> {
        static_assert(!std::is_integral<T>::value,
                      "Dividing by an integer value disallowed: would almost always produce 0");
        return make_quantity<Unit>(T{1} / x);
    }
};

//
// A mixin that enables scaling the units of a Quantity by multiplying or dividing.
//
template <template <typename U> class UnitWrapper, typename Unit>
struct ScalesQuantity {
    // (W * Q), for wrapper W and quantity Q.
    template <typename U, typename R>
    friend constexpr auto operator*(UnitWrapper<Unit>, Quantity<U, R> q) {
        return make_quantity<UnitProductT<Unit, U>>(q.in(U{}));
    }

    // (Q * W), for wrapper W and quantity Q.
    template <typename U, typename R>
    friend constexpr auto operator*(Quantity<U, R> q, UnitWrapper<Unit>) {
        return make_quantity<UnitProductT<U, Unit>>(q.in(U{}));
    }

    // (Q / W), for wrapper W and quantity Q.
    template <typename U, typename R>
    friend constexpr auto operator/(Quantity<U, R> q, UnitWrapper<Unit>) {
        return make_quantity<UnitQuotientT<U, Unit>>(q.in(U{}));
    }

    // (W / Q), for wrapper W and quantity Q.
    template <typename U, typename R>
    friend constexpr auto operator/(UnitWrapper<Unit>, Quantity<U, R> q) {
        static_assert(!std::is_integral<R>::value,
                      "Dividing by an integer value disallowed: would almost always produce 0");
        return make_quantity<UnitQuotientT<Unit, U>>(R{1} / q.in(U{}));
    }
};

// A mixin to compose `op(U, O)` into a new unit wrapper, for "main" wrapper `U` and "other" wrapper
// `O`.  (Implementation detail helper for `ComposesWith`.)
template <template <typename U> class UnitWrapper,
          typename Unit,
          template <typename U>
          class OtherWrapper,
          template <typename U>
          class ResultWrapper>
struct PrecomposesWith {
    // (U * O), for "main" wrapper U and "other" wrapper O.
    template <typename U>
    friend constexpr ResultWrapper<UnitProductT<Unit, U>> operator*(UnitWrapper<Unit>,
                                                                    OtherWrapper<U>) {
        return {};
    }

    // (U / O), for "main" wrapper U and "other" wrapper O.
    template <typename U>
    friend constexpr ResultWrapper<UnitQuotientT<Unit, U>> operator/(UnitWrapper<Unit>,
                                                                     OtherWrapper<U>) {
        return {};
    }
};

// A mixin to compose `op(O, U)` into a new unit wrapper, for "main" wrapper `U` and "other" wrapper
// `O`.  (Implementation detail helper for `ComposesWith`.)
template <template <typename U> class UnitWrapper,
          typename Unit,
          template <typename U>
          class OtherWrapper,
          template <typename U>
          class ResultWrapper>
struct PostcomposesWith {
    // (O * U), for "main" wrapper U and "other" wrapper O.
    template <typename U>
    friend constexpr ResultWrapper<UnitProductT<U, Unit>> operator*(OtherWrapper<U>,
                                                                    UnitWrapper<Unit>) {
        return {};
    }

    // (O / U), for "main" wrapper U and "other" wrapper O.
    template <typename U>
    friend constexpr ResultWrapper<UnitQuotientT<U, Unit>> operator/(OtherWrapper<U>,
                                                                     UnitWrapper<Unit>) {
        return {};
    }
};

// An empty version of `PostcomposesWith` for when `UnitWrapper` is the same as `OtherWrapper`.
// In this case, if we left it non-empty, the definitions would be ambiguous/redundant with the ones
// in `PrecoposesWith`.
template <template <typename U> class UnitWrapper,
          typename Unit,
          template <typename U>
          class ResultWrapper>
struct PostcomposesWith<UnitWrapper, Unit, UnitWrapper, ResultWrapper> {};

//
// A mixin to compose two unit wrappers into a new unit wrapper.
//
template <template <typename U> class UnitWrapper,
          typename Unit,
          template <typename U>
          class OtherWrapper,
          template <typename U>
          class ResultWrapper>
struct ComposesWith : PrecomposesWith<UnitWrapper, Unit, OtherWrapper, ResultWrapper>,
                      PostcomposesWith<UnitWrapper, Unit, OtherWrapper, ResultWrapper> {};

//
// A mixin to enable scaling a unit wrapper by a magnitude.
//
template <template <typename U> class UnitWrapper, typename Unit>
struct CanScaleByMagnitude {
    // (M * W), for magnitude M and wrapper W.
    template <typename... BPs>
    friend constexpr auto operator*(Magnitude<BPs...> m, UnitWrapper<Unit>) {
        return UnitWrapper<decltype(Unit{} * m)>{};
    }

    // (W * M), for magnitude M and wrapper W.
    template <typename... BPs>
    friend constexpr auto operator*(UnitWrapper<Unit>, Magnitude<BPs...> m) {
        return UnitWrapper<decltype(Unit{} * m)>{};
    }

    // (M / W), for magnitude M and wrapper W.
    template <typename... BPs>
    friend constexpr auto operator/(Magnitude<BPs...> m, UnitWrapper<Unit>) {
        return UnitWrapper<decltype(UnitInverseT<Unit>{} * m)>{};
    }

    // (W / M), for magnitude M and wrapper W.
    template <typename... BPs>
    friend constexpr auto operator/(UnitWrapper<Unit>, Magnitude<BPs...> m) {
        return UnitWrapper<decltype(Unit{} / m)>{};
    }
};

}  // namespace detail
}  // namespace au


namespace au {

// `QuantityPoint`: an _affine space type_ modeling points on a line.
//
// For a quick primer on affine space types, see: http://videocortex.io/2018/Affine-Space-Types/
//
// By "modeling points", we mean that `QuantityPoint` instances cannot be added to each other, and
// cannot be multiplied.  However, they can be subtracted: the difference between two
// `QuantityPoint` instances (of the same Unit) is a `Quantity` of that unit.  We can also add a
// `Quantity` to a `QuantityPoint`, and vice versa; the result is a new `QuantityPoint`.
//
// Key motivating examples include _mile markers_ (effectively `QuantityPoint<Miles, T>`), and
// _absolute temperature measurements_ (e.g., `QuantityPoint<Celsius, T>`).  This type is also
// analogous to `std::chrono::time_point`, in the same way that `Quantity` is analogous to
// `std::chrono::duration`.
template <typename UnitT, typename RepT>
class QuantityPoint;

template <typename UnitT>
struct QuantityPointMaker;

// Make a Quantity of the given Unit, which has this value as measured in the Unit.
template <typename UnitT, typename T>
constexpr auto make_quantity_point(T value) {
    return QuantityPointMaker<UnitT>{}(value);
}

// Trait to check whether two QuantityPoint types are exactly equivalent.
template <typename P1, typename P2>
struct AreQuantityPointTypesEquivalent;

namespace detail {
template <typename TargetRep, typename U1, typename U2>
struct OriginDisplacementFitsIn;
}  // namespace detail

// QuantityPoint implementation and API elaboration.
template <typename UnitT, typename RepT>
class QuantityPoint {
    // Q: When should we enable IMPLICIT construction from another QuantityPoint type?
    // A: EXACTLY WHEN our own Diff type can be IMPLICITLY constructed from the SUM of the target's
    //    Diff type, and the offset between our Units' zero points.
    //
    // In other words, there are two ways to fail implicit convertibility.
    //
    //   1. Their Diff type might not work with our Rep.  Examples:
    //      BAD: QuantityPoint<Milli<Meters>, int> -> QuantityPoint<Meters, int>
    //      OK : QuantityPoint<Kilo<Meters> , int> -> QuantityPoint<Meters, int>
    //
    //   2. Their zero point might be offset from ours by a non-representable amount.  Examples:
    //      BAD: QuantityPoint<Celsius, int> -> QuantityPoint<Kelvins, int>
    //      OK : QuantityPoint<Celsius, int> -> QuantityPoint<Kelvins, double>
    //      OK : QuantityPoint<Celsius, int> -> QuantityPoint<Milli<Kelvins>, int>
    template <typename OtherUnit, typename OtherRep>
    static constexpr bool should_enable_implicit_construction_from() {
        return std::is_convertible<
            decltype(std::declval<typename QuantityPoint<OtherUnit, OtherRep>::Diff>() +
                     origin_displacement(UnitT{}, OtherUnit{})),
            QuantityPoint::Diff>::value;
    }

    // This machinery exists to give us a conditionally explicit constructor, using SFINAE to select
    // the explicit or implicit version (https://stackoverflow.com/a/26949793/15777264).  If we had
    // C++20, we could use the `explicit(bool)` feature, making this code simpler and faster.
    template <bool ImplicitOk, typename OtherUnit, typename OtherRep>
    using EnableIfImplicitOkIs = std::enable_if_t<
        ImplicitOk ==
        QuantityPoint::should_enable_implicit_construction_from<OtherUnit, OtherRep>()>;

 public:
    using Rep = RepT;
    using Unit = UnitT;
    static constexpr Unit unit{};
    using Diff = Quantity<Unit, Rep>;

    // The default constructor produces a QuantityPoint in a valid but contractually unspecified
    // state.  It exists to give you an object you can assign to.  The main motivating factor for
    // including this is to support `std::atomic`, which requires its types to be
    // default-constructible.
    constexpr QuantityPoint() noexcept : x_{ZERO} {}

    template <typename OtherUnit,
              typename OtherRep,
              typename Enable = EnableIfImplicitOkIs<true, OtherUnit, OtherRep>>
    constexpr QuantityPoint(QuantityPoint<OtherUnit, OtherRep> other)  // NOLINT(runtime/explicit)
        : QuantityPoint{other.template as<Rep>(unit)} {}

    template <typename OtherUnit,
              typename OtherRep,
              typename Enable = EnableIfImplicitOkIs<false, OtherUnit, OtherRep>,
              typename ThisUnusedTemplateParameterDistinguishesUsFromTheAboveConstructor = void>
    // Deleted: use `.as<NewRep>(new_unit)` to force a cast.
    constexpr explicit QuantityPoint(QuantityPoint<OtherUnit, OtherRep> other) = delete;

    // The notion of "0" is *not* unambiguous for point types, because different scales can make
    // different decisions about what point is labeled as "0".
    constexpr QuantityPoint(Zero) = delete;

    template <typename NewRep,
              typename NewUnit,
              typename = std::enable_if_t<IsUnit<NewUnit>::value>>
    constexpr auto as(NewUnit u) const {
        return make_quantity_point<NewUnit>(this->template in<NewRep>(u));
    }

    template <typename NewUnit, typename = std::enable_if_t<IsUnit<NewUnit>::value>>
    constexpr auto as(NewUnit u) const {
        return make_quantity_point<NewUnit>(in(u));
    }

    template <typename NewRep,
              typename NewUnit,
              typename = std::enable_if_t<IsUnit<NewUnit>::value>>
    constexpr NewRep in(NewUnit u) const {
        return (rep_cast<NewRep>(x_) - rep_cast<NewRep>(OriginDisplacement<Unit, NewUnit>::value()))
            .template in<NewRep>(u);
    }

    template <typename NewUnit, typename = std::enable_if_t<IsUnit<NewUnit>::value>>
    constexpr Rep in(NewUnit u) const {
        static_assert(detail::OriginDisplacementFitsIn<Rep, NewUnit, Unit>::value,
                      "Cannot represent origin displacement in desired Rep");

        // `rep_cast` is needed because if these are integral types, their difference might become a
        // different type due to integer promotion.
        return rep_cast<Rep>(x_ + rep_cast<Rep>(OriginDisplacement<NewUnit, Unit>::value())).in(u);
    }

    // Overloads for passing a QuantityPointMaker.
    //
    // This is the "magic" that lets us write things like `position.in(meters_pt)`, instead of just
    // `position.in(Meters{})`.
    template <typename NewRep, typename NewUnit>
    constexpr auto as(QuantityPointMaker<NewUnit>) const {
        return as<NewRep>(NewUnit{});
    }
    template <typename NewUnit>
    constexpr auto as(QuantityPointMaker<NewUnit>) const {
        return as(NewUnit{});
    }
    template <typename NewRep, typename NewUnit>
    constexpr NewRep in(QuantityPointMaker<NewUnit>) const {
        return in<NewRep>(NewUnit{});
    }
    template <typename NewUnit>
    constexpr Rep in(QuantityPointMaker<NewUnit>) const {
        return in(NewUnit{});
    }

    // "Old-style" overloads with <U, R> template parameters, and no function parameters.
    //
    // Matches the syntax from the CppCon 2021 talk, and legacy Aurora usage.
    template <typename U>
    [[deprecated(
        "Do not write `.as<YourUnits>()`; write `.as(your_units)` instead.")]] constexpr auto
    as() const -> decltype(as(U{})) {
        return as(U{});
    }
    template <typename U, typename R, typename = std::enable_if_t<IsUnit<U>::value>>
    [[deprecated(
        "Do not write `.as<YourUnits, T>()`; write `.as<T>(your_units)` instead.")]] constexpr auto
    as() const {
        return as<R>(U{});
    }
    template <typename U>
    [[deprecated(
        "Do not write `.in<YourUnits>()`; write `.in(your_units)` instead.")]] constexpr auto
    in() const -> decltype(in(U{})) {
        return in(U{});
    }
    template <typename U, typename R, typename = std::enable_if_t<IsUnit<U>::value>>
    [[deprecated(
        "Do not write `.in<YourUnits, T>()`; write `.in<T>(your_units)` instead.")]] constexpr auto
    in() const {
        return in<R>(U{});
    }

    // "Forcing" conversions, which explicitly ignore safety checks for overflow and truncation.
    template <typename NewUnit>
    constexpr auto coerce_as(NewUnit) const {
        // Usage example: `p.coerce_as(new_units)`.
        return as<Rep>(NewUnit{});
    }
    template <typename NewRep, typename NewUnit>
    constexpr auto coerce_as(NewUnit) const {
        // Usage example: `p.coerce_as<T>(new_units)`.
        return as<NewRep>(NewUnit{});
    }
    template <typename NewUnit>
    constexpr auto coerce_in(NewUnit) const {
        // Usage example: `p.coerce_in(new_units)`.
        return in<Rep>(NewUnit{});
    }
    template <typename NewRep, typename NewUnit>
    constexpr auto coerce_in(NewUnit) const {
        // Usage example: `p.coerce_in<T>(new_units)`.
        return in<NewRep>(NewUnit{});
    }

    // Direct access to the underlying value member, with any Point-equivalent Unit.
    //
    // Mutable access, QuantityPointMaker input.
    template <typename U>
    Rep &data_in(const QuantityPointMaker<U> &) {
        static_assert(AreUnitsPointEquivalent<U, Unit>::value,
                      "Can only access value via Point-equivalent unit");
        return x_.data_in(QuantityMaker<U>{});
    }
    // Mutable access, Unit input.
    template <typename U>
    Rep &data_in(const U &) {
        return data_in(QuantityPointMaker<U>{});
    }
    // Const access, QuantityPointMaker input.
    template <typename U>
    const Rep &data_in(const QuantityPointMaker<U> &) const {
        static_assert(AreUnitsPointEquivalent<U, Unit>::value,
                      "Can only access value via Point-equivalent unit");
        return x_.data_in(QuantityMaker<U>{});
    }
    // Const access, Unit input.
    template <typename U>
    const Rep &data_in(const U &) const {
        return data_in(QuantityPointMaker<U>{});
    }

    // Comparison operators.
    constexpr friend bool operator==(QuantityPoint a, QuantityPoint b) { return a.x_ == b.x_; }
    constexpr friend bool operator!=(QuantityPoint a, QuantityPoint b) { return a.x_ != b.x_; }
    constexpr friend bool operator>=(QuantityPoint a, QuantityPoint b) { return a.x_ >= b.x_; }
    constexpr friend bool operator>(QuantityPoint a, QuantityPoint b) { return a.x_ > b.x_; }
    constexpr friend bool operator<=(QuantityPoint a, QuantityPoint b) { return a.x_ <= b.x_; }
    constexpr friend bool operator<(QuantityPoint a, QuantityPoint b) { return a.x_ < b.x_; }

    // Subtraction between two QuantityPoint types.
    constexpr friend Diff operator-(QuantityPoint a, QuantityPoint b) { return a.x_ - b.x_; }

    // Left and right addition of a Diff.
    constexpr friend auto operator+(Diff d, QuantityPoint p) { return QuantityPoint{d + p.x_}; }
    constexpr friend auto operator+(QuantityPoint p, Diff d) { return QuantityPoint{p.x_ + d}; }

    // Right subtraction of a Diff.
    constexpr friend auto operator-(QuantityPoint p, Diff d) { return QuantityPoint{p.x_ - d}; }

    // Short-hand addition assignment.
    constexpr QuantityPoint &operator+=(Diff diff) {
        x_ += diff;
        return *this;
    }

    // Short-hand subtraction assignment.
    constexpr QuantityPoint &operator-=(Diff diff) {
        x_ -= diff;
        return *this;
    }

    // Permit this factory functor to access our private constructor.
    //
    // We allow this because it explicitly names the unit at the callsite, even if people refer to
    // this present Quantity type by an alias that omits the unit.  This preserves Unit Safety and
    // promotes callsite readability.
    friend struct QuantityPointMaker<Unit>;

 private:
    constexpr explicit QuantityPoint(Diff x) : x_{x} {}

    Diff x_;
};

template <typename Unit>
struct QuantityPointMaker {
    static constexpr auto unit = Unit{};

    template <typename T>
    constexpr auto operator()(T value) const {
        return QuantityPoint<Unit, T>{make_quantity<Unit>(value)};
    }

    template <typename... BPs>
    constexpr auto operator*(Magnitude<BPs...> m) const {
        return QuantityPointMaker<decltype(unit * m)>{};
    }

    template <typename... BPs>
    constexpr auto operator/(Magnitude<BPs...> m) const {
        return QuantityPointMaker<decltype(unit / m)>{};
    }
};

// Type trait to detect whether two QuantityPoint types are equivalent.
//
// In this library, QuantityPoint types are "equivalent" exactly when they use the same Rep, and are
// based on point-equivalent units.
template <typename U1, typename U2, typename R1, typename R2>
struct AreQuantityPointTypesEquivalent<QuantityPoint<U1, R1>, QuantityPoint<U2, R2>>
    : stdx::conjunction<std::is_same<R1, R2>, AreUnitsPointEquivalent<U1, U2>> {};

// Cast QuantityPoint to a different underlying type.
template <typename NewRep, typename Unit, typename Rep>
constexpr auto rep_cast(QuantityPoint<Unit, Rep> q) {
    return q.template as<NewRep>(Unit{});
}

//
// QuantityPoint aliases to set a particular Rep.
//
// This presents a less cumbersome interface for end users.
//
template <typename UnitT>
using QuantityPointD = QuantityPoint<UnitT, double>;
template <typename UnitT>
using QuantityPointF = QuantityPoint<UnitT, float>;
template <typename UnitT>
using QuantityPointI = QuantityPoint<UnitT, int>;
template <typename UnitT>
using QuantityPointU = QuantityPoint<UnitT, unsigned int>;
template <typename UnitT>
using QuantityPointI32 = QuantityPoint<UnitT, int32_t>;
template <typename UnitT>
using QuantityPointU32 = QuantityPoint<UnitT, uint32_t>;
template <typename UnitT>
using QuantityPointI64 = QuantityPoint<UnitT, int64_t>;
template <typename UnitT>
using QuantityPointU64 = QuantityPoint<UnitT, uint64_t>;

namespace detail {
template <typename X, typename Y, typename Func>
constexpr auto using_common_point_unit(X x, Y y, Func f) {
    using R = std::common_type_t<typename X::Rep, typename Y::Rep>;
    constexpr auto u = CommonPointUnitT<typename X::Unit, typename Y::Unit>{};
    return f(rep_cast<R>(x).as(u), rep_cast<R>(y).as(u));
}
}  // namespace detail

// Comparison functions for compatible QuantityPoint types.
template <typename U1, typename U2, typename R1, typename R2>
constexpr auto operator<(QuantityPoint<U1, R1> p1, QuantityPoint<U2, R2> p2) {
    return detail::using_common_point_unit(p1, p2, detail::less);
}
template <typename U1, typename U2, typename R1, typename R2>
constexpr auto operator>(QuantityPoint<U1, R1> p1, QuantityPoint<U2, R2> p2) {
    return detail::using_common_point_unit(p1, p2, detail::greater);
}
template <typename U1, typename U2, typename R1, typename R2>
constexpr auto operator<=(QuantityPoint<U1, R1> p1, QuantityPoint<U2, R2> p2) {
    return detail::using_common_point_unit(p1, p2, detail::less_equal);
}
template <typename U1, typename U2, typename R1, typename R2>
constexpr auto operator>=(QuantityPoint<U1, R1> p1, QuantityPoint<U2, R2> p2) {
    return detail::using_common_point_unit(p1, p2, detail::greater_equal);
}
template <typename U1, typename U2, typename R1, typename R2>
constexpr auto operator==(QuantityPoint<U1, R1> p1, QuantityPoint<U2, R2> p2) {
    return detail::using_common_point_unit(p1, p2, detail::equal);
}
template <typename U1, typename U2, typename R1, typename R2>
constexpr auto operator!=(QuantityPoint<U1, R1> p1, QuantityPoint<U2, R2> p2) {
    return detail::using_common_point_unit(p1, p2, detail::not_equal);
}

namespace detail {
// Another subtlety arises when we mix QuantityPoint and Quantity in adding or subtracting.  We
// actually don't want to use `CommonPointUnitT`, because this is too restrictive if the units have
// different origins.  Imagine adding a `Quantity<Kelvins>` to a `QuantityPoint<Celsius>`---we
// wouldn't want this to subdivide the unit of measure to satisfy an additive relative offset which
// we will never actually use!
//
// The solution is to set the (unused!) origin of the `Quantity` unit to the same as the
// `QuantityPoint` unit.  Once we do, everything flows simply from there.
//
// This utility should be used for every overload below which combines a `QuantityPoint` with a
// `Quantity`.
template <typename Target, typename U>
constexpr auto borrow_origin(U u) {
    return Target{} * unit_ratio(u, Target{});
}
}  // namespace detail

// Addition and subtraction functions for compatible QuantityPoint types.
template <typename UnitP, typename UnitQ, typename RepP, typename RepQ>
constexpr auto operator+(QuantityPoint<UnitP, RepP> p, Quantity<UnitQ, RepQ> q) {
    constexpr auto new_unit_q = detail::borrow_origin<UnitP>(UnitQ{});
    return detail::using_common_point_unit(p, q.as(new_unit_q), detail::plus);
}
template <typename UnitQ, typename UnitP, typename RepQ, typename RepP>
constexpr auto operator+(Quantity<UnitQ, RepQ> q, QuantityPoint<UnitP, RepP> p) {
    constexpr auto new_unit_q = detail::borrow_origin<UnitP>(UnitQ{});
    return detail::using_common_point_unit(q.as(new_unit_q), p, detail::plus);
}
template <typename UnitP, typename UnitQ, typename R1, typename RepQ>
constexpr auto operator-(QuantityPoint<UnitP, R1> p, Quantity<UnitQ, RepQ> q) {
    constexpr auto new_unit_q = detail::borrow_origin<UnitP>(UnitQ{});
    return detail::using_common_point_unit(p, q.as(new_unit_q), detail::minus);
}
template <typename U1, typename U2, typename R1, typename R2>
constexpr auto operator-(QuantityPoint<U1, R1> p1, QuantityPoint<U2, R2> p2) {
    return detail::using_common_point_unit(p1, p2, detail::minus);
}

namespace detail {

template <typename TargetRep, typename U, typename R>
constexpr bool underlying_value_in_range(Quantity<U, R> q) {
    return stdx::in_range<TargetRep>(q.in(U{}));
}

template <typename TargetRep>
constexpr bool underlying_value_in_range(Zero) {
    return true;
}

template <typename TargetRep, typename U1, typename U2>
struct OriginDisplacementFitsIn
    : std::conditional_t<std::is_integral<TargetRep>::value,
                         stdx::bool_constant<underlying_value_in_range<TargetRep>(
                             OriginDisplacement<U1, U2>::value())>,
                         std::true_type> {};
}  // namespace detail
}  // namespace au


namespace au {

//
// A representation of the symbol for a unit.
//
// To use, create an instance variable templated on a unit, and make the instance variable's name
// the symbol to represent.  For example:
//
//     constexpr auto m = SymbolFor<Meters>{};
//
template <typename Unit>
struct SymbolFor : detail::MakesQuantityFromNumber<SymbolFor, Unit>,
                   detail::ScalesQuantity<SymbolFor, Unit>,
                   detail::ComposesWith<SymbolFor, Unit, SymbolFor, SymbolFor> {};

//
// Create a unit symbol using the more fluent APIs that unit slots make possible.  For example:
//
//     constexpr auto mps = symbol_for(meters / second);
//
// This is generally easier to work with and makes code that is easier to read, at the cost of being
// (very slightly) slower to compile.
//
template <typename UnitSlot>
constexpr auto symbol_for(UnitSlot) {
    return SymbolFor<AssociatedUnitT<UnitSlot>>{};
}

// Support using symbols in unit slot APIs (e.g., `v.in(m / s)`).
template <typename U>
struct AssociatedUnit<SymbolFor<U>> : stdx::type_identity<U> {};

}  // namespace au


namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/main/howto/new-units/).
template <typename T>
struct RadiansLabel {
    static constexpr const char label[] = "rad";
};
template <typename T>
constexpr const char RadiansLabel<T>::label[];
struct Radians : UnitImpl<Angle>, RadiansLabel<void> {
    using RadiansLabel<void>::label;
};
constexpr auto radian = SingularNameFor<Radians>{};
constexpr auto radians = QuantityMaker<Radians>{};

namespace symbols {
constexpr auto rad = SymbolFor<Radians>{};
}
}  // namespace au


namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/main/howto/new-units/).
template <typename T>
struct CandelasLabel {
    static constexpr const char label[] = "cd";
};
template <typename T>
constexpr const char CandelasLabel<T>::label[];
struct Candelas : UnitImpl<LuminousIntensity>, CandelasLabel<void> {
    using CandelasLabel<void>::label;
};
constexpr auto candela = SingularNameFor<Candelas>{};
constexpr auto candelas = QuantityMaker<Candelas>{};

namespace symbols {
constexpr auto cd = SymbolFor<Candelas>{};
}
}  // namespace au


namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/main/howto/new-units/).
template <typename T>
struct MolesLabel {
    static constexpr const char label[] = "mol";
};
template <typename T>
constexpr const char MolesLabel<T>::label[];
struct Moles : UnitImpl<AmountOfSubstance>, MolesLabel<void> {
    using MolesLabel<void>::label;
};
constexpr auto mole = SingularNameFor<Moles>{};
constexpr auto moles = QuantityMaker<Moles>{};

namespace symbols {
constexpr auto mol = SymbolFor<Moles>{};
}
}  // namespace au


namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/main/howto/new-units/).
template <typename T>
struct AmperesLabel {
    static constexpr const char label[] = "A";
};
template <typename T>
constexpr const char AmperesLabel<T>::label[];
struct Amperes : UnitImpl<Current>, AmperesLabel<void> {
    using AmperesLabel<void>::label;
};
constexpr auto ampere = SingularNameFor<Amperes>{};
constexpr auto amperes = QuantityMaker<Amperes>{};

namespace symbols {
constexpr auto A = SymbolFor<Amperes>{};
}

}  // namespace au


namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/main/howto/new-units/).
template <typename T>
struct KelvinsLabel {
    static constexpr const char label[] = "K";
};
template <typename T>
constexpr const char KelvinsLabel<T>::label[];
struct Kelvins : UnitImpl<Temperature>, KelvinsLabel<void> {
    using KelvinsLabel<void>::label;
};
constexpr auto kelvin = SingularNameFor<Kelvins>{};
constexpr auto kelvins = QuantityMaker<Kelvins>{};
constexpr auto kelvins_pt = QuantityPointMaker<Kelvins>{};

namespace symbols {
constexpr auto K = SymbolFor<Kelvins>{};
}
}  // namespace au


namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/main/howto/new-units/).
template <typename T>
struct GramsLabel {
    static constexpr const char label[] = "g";
};
template <typename T>
constexpr const char GramsLabel<T>::label[];
struct Grams : UnitImpl<Mass>, GramsLabel<void> {
    using GramsLabel<void>::label;
};
constexpr auto gram = SingularNameFor<Grams>{};
constexpr auto grams = QuantityMaker<Grams>{};

namespace symbols {
constexpr auto g = SymbolFor<Grams>{};
}
}  // namespace au


namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/main/howto/new-units/).
template <typename T>
struct SecondsLabel {
    static constexpr const char label[] = "s";
};
template <typename T>
constexpr const char SecondsLabel<T>::label[];
struct Seconds : UnitImpl<Time>, SecondsLabel<void> {
    using SecondsLabel<void>::label;
};
constexpr auto second = SingularNameFor<Seconds>{};
constexpr auto seconds = QuantityMaker<Seconds>{};

namespace symbols {
constexpr auto s = SymbolFor<Seconds>{};
}
}  // namespace au


namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/main/howto/new-units/).
template <typename T>
struct MetersLabel {
    static constexpr const char label[] = "m";
};
template <typename T>
constexpr const char MetersLabel<T>::label[];
struct Meters : UnitImpl<Length>, MetersLabel<void> {
    using MetersLabel<void>::label;
};
constexpr auto meter = SingularNameFor<Meters>{};
constexpr auto meters = QuantityMaker<Meters>{};
constexpr auto meters_pt = QuantityPointMaker<Meters>{};

namespace symbols {
constexpr auto m = SymbolFor<Meters>{};
}
}  // namespace au


namespace au {

template <template <class U> class Prefix>
struct PrefixApplier {
    // Applying a Prefix to a Unit instance, creates an instance of the Prefixed Unit.
    template <typename U>
    constexpr auto operator()(U) const {
        return Prefix<U>{};
    }

    // Applying a Prefix to a QuantityMaker instance, creates a maker for the Prefixed Unit.
    template <typename U>
    constexpr auto operator()(QuantityMaker<U>) const {
        return QuantityMaker<Prefix<U>>{};
    }

    // Applying a Prefix to a QuantityPointMaker instance, changes it to make the Prefixed Unit.
    template <typename U>
    constexpr auto operator()(QuantityPointMaker<U>) const {
        return QuantityPointMaker<Prefix<U>>{};
    }

    // Applying a Prefix to a SingularNameFor instance, creates a singularly-named instance of the
    // Prefixed Unit.
    template <typename U>
    constexpr auto operator()(SingularNameFor<U>) const {
        return SingularNameFor<Prefix<U>>{};
    }

    // Applying a Prefix to a SymbolFor instance, creates a symbolically-named instance of the
    // Prefixed unit.
    template <typename U>
    constexpr auto operator()(SymbolFor<U>) const {
        return SymbolFor<Prefix<U>>{};
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// SI Prefixes.

template <typename U>
struct Quetta : decltype(U{} * pow<30>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("Q", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Quetta<U>::label;
constexpr auto quetta = PrefixApplier<Quetta>{};

template <typename U>
struct Ronna : decltype(U{} * pow<27>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("R", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Ronna<U>::label;
constexpr auto ronna = PrefixApplier<Ronna>{};

template <typename U>
struct Yotta : decltype(U{} * pow<24>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("Y", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Yotta<U>::label;
constexpr auto yotta = PrefixApplier<Yotta>{};

template <typename U>
struct Zetta : decltype(U{} * pow<21>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("Z", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Zetta<U>::label;
constexpr auto zetta = PrefixApplier<Zetta>{};

template <typename U>
struct Exa : decltype(U{} * pow<18>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("E", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Exa<U>::label;
constexpr auto exa = PrefixApplier<Exa>{};

template <typename U>
struct Peta : decltype(U{} * pow<15>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("P", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Peta<U>::label;
constexpr auto peta = PrefixApplier<Peta>{};

template <typename U>
struct Tera : decltype(U{} * pow<12>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("T", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Tera<U>::label;
constexpr auto tera = PrefixApplier<Tera>{};

template <typename U>
struct Giga : decltype(U{} * pow<9>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("G", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Giga<U>::label;
constexpr auto giga = PrefixApplier<Giga>{};

template <typename U>
struct Mega : decltype(U{} * pow<6>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("M", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Mega<U>::label;
constexpr auto mega = PrefixApplier<Mega>{};

template <typename U>
struct Kilo : decltype(U{} * pow<3>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("k", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Kilo<U>::label;
constexpr auto kilo = PrefixApplier<Kilo>{};

template <typename U>
struct Hecto : decltype(U{} * pow<2>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("h", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Hecto<U>::label;
constexpr auto hecto = PrefixApplier<Hecto>{};

template <typename U>
struct Deka : decltype(U{} * pow<1>(mag<10>())) {
    static constexpr detail::ExtendedLabel<2, U> label = detail::concatenate("da", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<2, U> Deka<U>::label;
constexpr auto deka = PrefixApplier<Deka>{};

template <typename U>
struct Deci : decltype(U{} * pow<-1>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("d", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Deci<U>::label;
constexpr auto deci = PrefixApplier<Deci>{};

template <typename U>
struct Centi : decltype(U{} * pow<-2>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("c", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Centi<U>::label;
constexpr auto centi = PrefixApplier<Centi>{};

template <typename U>
struct Milli : decltype(U{} * pow<-3>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("m", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Milli<U>::label;
constexpr auto milli = PrefixApplier<Milli>{};

template <typename U>
struct Micro : decltype(U{} * pow<-6>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("u", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Micro<U>::label;
constexpr auto micro = PrefixApplier<Micro>{};

template <typename U>
struct Nano : decltype(U{} * pow<-9>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("n", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Nano<U>::label;
constexpr auto nano = PrefixApplier<Nano>{};

template <typename U>
struct Pico : decltype(U{} * pow<-12>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("p", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Pico<U>::label;
constexpr auto pico = PrefixApplier<Pico>{};

template <typename U>
struct Femto : decltype(U{} * pow<-15>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("f", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Femto<U>::label;
constexpr auto femto = PrefixApplier<Femto>{};

template <typename U>
struct Atto : decltype(U{} * pow<-18>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("a", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Atto<U>::label;
constexpr auto atto = PrefixApplier<Atto>{};

template <typename U>
struct Zepto : decltype(U{} * pow<-21>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("z", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Zepto<U>::label;
constexpr auto zepto = PrefixApplier<Zepto>{};

template <typename U>
struct Yocto : decltype(U{} * pow<-24>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("y", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Yocto<U>::label;
constexpr auto yocto = PrefixApplier<Yocto>{};

template <typename U>
struct Ronto : decltype(U{} * pow<-27>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("r", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Ronto<U>::label;
constexpr auto ronto = PrefixApplier<Ronto>{};

template <typename U>
struct Quecto : decltype(U{} * pow<-30>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("q", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Quecto<U>::label;
constexpr auto quecto = PrefixApplier<Quecto>{};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Binary Prefixes.

template <typename U>
struct Yobi : decltype(U{} * pow<80>(mag<2>())) {
    static constexpr detail::ExtendedLabel<2, U> label = detail::concatenate("Yi", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<2, U> Yobi<U>::label;
constexpr auto yobi = PrefixApplier<Yobi>{};

template <typename U>
struct Zebi : decltype(U{} * pow<70>(mag<2>())) {
    static constexpr detail::ExtendedLabel<2, U> label = detail::concatenate("Zi", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<2, U> Zebi<U>::label;
constexpr auto zebi = PrefixApplier<Zebi>{};

template <typename U>
struct Exbi : decltype(U{} * pow<60>(mag<2>())) {
    static constexpr detail::ExtendedLabel<2, U> label = detail::concatenate("Ei", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<2, U> Exbi<U>::label;
constexpr auto exbi = PrefixApplier<Exbi>{};

template <typename U>
struct Pebi : decltype(U{} * pow<50>(mag<2>())) {
    static constexpr detail::ExtendedLabel<2, U> label = detail::concatenate("Pi", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<2, U> Pebi<U>::label;
constexpr auto pebi = PrefixApplier<Pebi>{};

template <typename U>
struct Tebi : decltype(U{} * pow<40>(mag<2>())) {
    static constexpr detail::ExtendedLabel<2, U> label = detail::concatenate("Ti", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<2, U> Tebi<U>::label;
constexpr auto tebi = PrefixApplier<Tebi>{};

template <typename U>
struct Gibi : decltype(U{} * pow<30>(mag<2>())) {
    static constexpr detail::ExtendedLabel<2, U> label = detail::concatenate("Gi", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<2, U> Gibi<U>::label;
constexpr auto gibi = PrefixApplier<Gibi>{};

template <typename U>
struct Mebi : decltype(U{} * pow<20>(mag<2>())) {
    static constexpr detail::ExtendedLabel<2, U> label = detail::concatenate("Mi", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<2, U> Mebi<U>::label;
constexpr auto mebi = PrefixApplier<Mebi>{};

template <typename U>
struct Kibi : decltype(U{} * pow<10>(mag<2>())) {
    static constexpr detail::ExtendedLabel<2, U> label = detail::concatenate("Ki", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<2, U> Kibi<U>::label;
constexpr auto kibi = PrefixApplier<Kibi>{};

}  // namespace au


namespace au {

//
// A monovalue type to represent a constant value, including its units, if any.
//
// Users can multiply or divide `Constant` instances by raw numbers or `Quantity` instances, and it
// will perform symbolic arithmetic at compile time without affecting the stored numeric value.
// `Constant` also composes with other constants, and with `QuantityMaker` and other related types.
//
// Although `Constant` does not have any specific numeric type associated with it (as opposed to
// `Quantity`), it can easily convert to any appropriate `Quantity` type, with any rep.  Unlike
// `Quantity`, these conversions support _exact_ safety checks, so that every conversion producing a
// correctly representable value will succeed, and every unrepresentable conversion will fail.
//
template <typename Unit>
struct Constant : detail::MakesQuantityFromNumber<Constant, Unit>,
                  detail::ScalesQuantity<Constant, Unit>,
                  detail::ComposesWith<Constant, Unit, Constant, Constant>,
                  detail::ComposesWith<Constant, Unit, QuantityMaker, QuantityMaker>,
                  detail::ComposesWith<Constant, Unit, SingularNameFor, SingularNameFor>,
                  detail::CanScaleByMagnitude<Constant, Unit> {
    // Convert this constant to a Quantity of the given rep.
    template <typename T>
    constexpr auto as() const {
        return make_quantity<Unit>(static_cast<T>(1));
    }

    // Convert this constant to a Quantity of the given unit and rep, ignoring safety checks.
    template <typename T, typename OtherUnit>
    constexpr auto coerce_as(OtherUnit u) const {
        return as<T>().coerce_as(u);
    }

    // Convert this constant to a Quantity of the given unit and rep.
    template <typename T, typename OtherUnit>
    constexpr auto as(OtherUnit u) const {
        static_assert(can_store_value_in<T>(u), "Cannot represent constant in this unit/rep");
        return coerce_as<T>(u);
    }

    // Get the value of this constant in the given unit and rep, ignoring safety checks.
    template <typename T, typename OtherUnit>
    constexpr auto coerce_in(OtherUnit u) const {
        return as<T>().coerce_in(u);
    }

    // Get the value of this constant in the given unit and rep.
    template <typename T, typename OtherUnit>
    constexpr auto in(OtherUnit u) const {
        static_assert(can_store_value_in<T>(u), "Cannot represent constant in this unit/rep");
        return coerce_in<T>(u);
    }

    // Implicitly convert to any quantity type which passes safety checks.
    template <typename U, typename R>
    constexpr operator Quantity<U, R>() const {
        return as<R>(U{});
    }

    // Static function to check whether this constant can be exactly-represented in the given rep
    // `T` and unit `OtherUnit`.
    template <typename T, typename OtherUnit>
    static constexpr bool can_store_value_in(OtherUnit other) {
        return representable_in<T>(unit_ratio(Unit{}, other));
    }

    // Implicitly convert to type with an exactly corresponding quantity that passes safety checks.
    template <
        typename T,
        typename = std::enable_if_t<can_store_value_in<typename CorrespondingQuantity<T>::Rep>(
            typename CorrespondingQuantity<T>::Unit{})>>
    constexpr operator T() const {
        return as<typename CorrespondingQuantity<T>::Rep>(
            typename CorrespondingQuantity<T>::Unit{});
    }
};

// Make a constant from the given unit.
//
// Note that the argument is a _unit slot_, and thus can also accept things like `QuantityMaker` and
// `SymbolFor` in addition to regular units.
template <typename UnitSlot>
constexpr Constant<AssociatedUnitT<UnitSlot>> make_constant(UnitSlot) {
    return {};
}

// Support using `Constant` in a unit slot.
template <typename Unit>
struct AssociatedUnit<Constant<Unit>> : stdx::type_identity<Unit> {};

}  // namespace au



namespace au {

// Define 1:1 mapping between duration types of chrono library and our library.
template <typename RepT, typename Period>
struct CorrespondingQuantity<std::chrono::duration<RepT, Period>> {
    using Unit = decltype(Seconds{} * (mag<Period::num>() / mag<Period::den>()));
    using Rep = RepT;

    using ChronoDuration = std::chrono::duration<Rep, Period>;

    static constexpr Rep extract_value(ChronoDuration d) { return d.count(); }
    static constexpr ChronoDuration construct_from_value(Rep x) { return ChronoDuration{x}; }
};

// Convert any Au duration quantity to an equivalent `std::chrono::duration`.
template <typename U, typename R>
constexpr auto as_chrono_duration(Quantity<U, R> dt) {
    constexpr auto ratio = unit_ratio(U{}, seconds);
    static_assert(is_rational(ratio), "Cannot convert to chrono::duration with non-rational ratio");
    return std::chrono::duration<R,
                                 std::ratio<get_value<std::intmax_t>(numerator(ratio)),
                                            get_value<std::intmax_t>(denominator(ratio))>>{dt};
}

}  // namespace au



namespace au {

// Streaming output support for Quantity types.
template <typename U, typename R>
std::ostream &operator<<(std::ostream &out, const Quantity<U, R> &q) {
    // In the case that the Rep is a type that resolves to 'char' (e.g. int8_t),
    // the << operator will match the implementation that takes a character
    // literal.  Using the unary + operator will trigger an integer promotion on
    // the operand, which will then match an appropriate << operator that will
    // output the integer representation.
    out << +q.in(U{}) << " " << unit_label(U{});
    return out;
}

// Streaming output support for QuantityPoint types.
template <typename U, typename R>
std::ostream &operator<<(std::ostream &out, const QuantityPoint<U, R> &p) {
    out << "@(" << (p - rep_cast<R>(make_quantity_point<U>(0))) << ")";
    return out;
}

// Streaming output support for Zero.  (Useful for printing in unit test failures.)
inline std::ostream &operator<<(std::ostream &out, Zero) {
    out << "0";
    return out;
}

}  // namespace au


namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/main/howto/new-units/).
template <typename T>
struct BitsLabel {
    static constexpr const char label[] = "b";
};
template <typename T>
constexpr const char BitsLabel<T>::label[];
struct Bits : UnitImpl<Information>, BitsLabel<void> {
    using BitsLabel<void>::label;
};
constexpr auto bit = SingularNameFor<Bits>{};
constexpr auto bits = QuantityMaker<Bits>{};

namespace symbols {
constexpr auto b = SymbolFor<Bits>{};
}
}  // namespace au



namespace au {

// If we don't provide these, then unqualified uses of `sin()`, etc. from <cmath> will break.  Name
// Lookup will stop once it hits `::au::sin()`, hiding the `::sin()` overload in the global
// namespace.  To learn more about Name Lookup, see this article (https://abseil.io/tips/49).
using std::abs;
using std::copysign;
using std::cos;
using std::fmod;
using std::isnan;
using std::max;
using std::min;
using std::remainder;
using std::sin;
using std::sqrt;
using std::tan;

namespace detail {

// This utility handles converting Quantity to Radians in a uniform way, while also giving a more
// direct error message via the static_assert if users make a coding error and pass the wrong type.
template <typename U, typename R>
auto in_radians(Quantity<U, R> q) {
    static_assert(HasSameDimension<U, Radians>{},
                  "Can only use trig functions with Angle-dimensioned Quantity instances");

    // The standard library trig functions handle types as follows:
    // - For floating point inputs, return the same type as the input.
    // - For integral inputs, cast to a `double` and return a `double`.
    // See, for instance: https://en.cppreference.com/w/cpp/numeric/math/sin
    using PromotedT = std::conditional_t<std::is_floating_point<R>::value, R, double>;

    return q.template in<PromotedT>(radians);
}

template <typename T>
constexpr T int_pow_impl(T x, int exp) {
    if (exp < 0) {
        return T{1} / int_pow_impl(x, -exp);
    }

    if (exp == 0) {
        return T{1};
    }

    if (exp % 2 == 1) {
        return x * int_pow_impl(x, exp - 1);
    }

    const auto root = int_pow_impl(x, exp / 2);
    return root * root;
}

// Rounding a Quantity by a function `f()` (where `f` could be `std::round`, `std::ceil`, or
// `std::floor`) can require _two_ steps: unit conversion, and type conversion.  The unit conversion
// risks truncating the value if R is an integral type!  To prevent this, when we do the unit
// conversion, we use "whatever Rep `f()` would produce," because that is always a floating point
// type.
//
// This risks breaking the correspondence between the Rep of our Quantity, and the output type of
// `f()`.  For that correspondence to be _preserved_, we would need to make sure that
// `f(RoundingRepT{})` returns the same type as `f(R{})`.  We believe this is always the case based
// on the documentation:
//
// https://en.cppreference.com/w/cpp/numeric/math/round
// https://en.cppreference.com/w/cpp/numeric/math/floor
// https://en.cppreference.com/w/cpp/numeric/math/ceil
//
// Both of these assumptions---that our RoundingRep is floating point, and that it doesn't change
// the output Rep type---we verify via `static_assert`.
template <typename Q, typename RoundingUnits>
struct RoundingRep;
template <typename Q, typename RoundingUnits>
using RoundingRepT = typename RoundingRep<Q, RoundingUnits>::type;
template <typename U, typename R, typename RoundingUnits>
struct RoundingRep<Quantity<U, R>, RoundingUnits> {
    using type = decltype(std::round(R{}));

    // Test our floating point assumption.
    static_assert(std::is_floating_point<type>::value, "");

    // Test our type identity assumption, for every function which is a client of this utility.
    static_assert(std::is_same<decltype(std::round(type{})), decltype(std::round(R{}))>::value, "");
    static_assert(std::is_same<decltype(std::floor(type{})), decltype(std::floor(R{}))>::value, "");
    static_assert(std::is_same<decltype(std::ceil(type{})), decltype(std::ceil(R{}))>::value, "");
};
}  // namespace detail

// The absolute value of a Quantity.
template <typename U, typename R>
auto abs(Quantity<U, R> q) {
    return make_quantity<U>(std::abs(q.in(U{})));
}

// Wrapper for std::acos() which returns strongly typed angle quantity.
template <typename T>
auto arccos(T x) {
    return radians(std::acos(x));
}

// Wrapper for std::asin() which returns strongly typed angle quantity.
template <typename T>
auto arcsin(T x) {
    return radians(std::asin(x));
}

// Wrapper for std::atan() which returns strongly typed angle quantity.
template <typename T>
auto arctan(T x) {
    return radians(std::atan(x));
}

// Wrapper for std::atan2() which returns strongly typed angle quantity.
template <typename T, typename U>
auto arctan2(T y, U x) {
    return radians(std::atan2(y, x));
}

// arctan2() overload which supports same-dimensioned Quantity types.
template <typename U1, typename R1, typename U2, typename R2>
auto arctan2(Quantity<U1, R1> y, Quantity<U2, R2> x) {
    constexpr auto common_unit = CommonUnitT<U1, U2>{};
    return arctan2(y.in(common_unit), x.in(common_unit));
}

// Clamp the first quantity to within the range of the second two.
template <typename UV, typename ULo, typename UHi, typename RV, typename RLo, typename RHi>
constexpr auto clamp(Quantity<UV, RV> v, Quantity<ULo, RLo> lo, Quantity<UHi, RHi> hi) {
    using U = CommonUnitT<UV, ULo, UHi>;
    using R = std::common_type_t<RV, RLo, RHi>;
    using ResultT = Quantity<U, R>;
    return (v < lo) ? ResultT{lo} : (hi < v) ? ResultT{hi} : ResultT{v};
}

// `clamp` overloads for when either boundary is `Zero`.
//
// NOTE: these will not work if _both_ boundaries are `Zero`, or if the quantity being clamped is
// `Zero`.  We do not think these use cases are very useful, but we're open to revisiting this if we
// receive a persuasive argument otherwise.
template <typename UV, typename UHi, typename RV, typename RHi>
constexpr auto clamp(Quantity<UV, RV> v, Zero z, Quantity<UHi, RHi> hi) {
    using U = CommonUnitT<UV, UHi>;
    using R = std::common_type_t<RV, RHi>;
    using ResultT = Quantity<U, R>;
    return (v < z) ? ResultT{z} : (hi < v) ? ResultT{hi} : ResultT{v};
}
template <typename UV, typename ULo, typename RV, typename RLo>
constexpr auto clamp(Quantity<UV, RV> v, Quantity<ULo, RLo> lo, Zero z) {
    using U = CommonUnitT<UV, ULo>;
    using R = std::common_type_t<RV, RLo>;
    using ResultT = Quantity<U, R>;
    return (v < lo) ? ResultT{lo} : (z < v) ? ResultT{z} : ResultT{v};
}

// Clamp the first point to within the range of the second two.
template <typename UV, typename ULo, typename UHi, typename RV, typename RLo, typename RHi>
constexpr auto clamp(QuantityPoint<UV, RV> v,
                     QuantityPoint<ULo, RLo> lo,
                     QuantityPoint<UHi, RHi> hi) {
    using U = CommonPointUnitT<UV, ULo, UHi>;
    using R = std::common_type_t<RV, RLo, RHi>;
    using ResultT = QuantityPoint<U, R>;
    return (v < lo) ? ResultT{lo} : (hi < v) ? ResultT{hi} : ResultT{v};
}

// Copysign where the magnitude has units.
template <typename U, typename R, typename T>
constexpr auto copysign(Quantity<U, R> mag, T sgn) {
    return make_quantity<U>(std::copysign(mag.in(U{}), sgn));
}

// Copysign where the sign has units.
template <typename T, typename U, typename R>
constexpr auto copysign(T mag, Quantity<U, R> sgn) {
    return std::copysign(mag, sgn.in(U{}));
}

// Copysign where both the magnitude and sign have units (disambiguates between the above).
template <typename U1, typename R1, typename U2, typename R2>
constexpr auto copysign(Quantity<U1, R1> mag, Quantity<U2, R2> sgn) {
    return make_quantity<U1>(std::copysign(mag.in(U1{}), sgn.in(U2{})));
}

// Wrapper for std::cos() which accepts a strongly typed angle quantity.
template <typename U, typename R>
auto cos(Quantity<U, R> q) {
    return std::cos(detail::in_radians(q));
}

// The floating point remainder of two values of the same dimension.
template <typename U1, typename R1, typename U2, typename R2>
auto fmod(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    using U = CommonUnitT<U1, U2>;
    using R = decltype(std::fmod(R1{}, R2{}));
    return make_quantity<U>(std::fmod(q1.template in<R>(U{}), q2.template in<R>(U{})));
}

// Raise a Quantity to an integer power.
template <int Exp, typename U, typename R>
constexpr auto int_pow(Quantity<U, R> q) {
    static_assert((!std::is_integral<R>::value) || (Exp >= 0),
                  "Negative exponent on integral represented units are not supported.");

    return make_quantity<UnitPowerT<U, Exp>>(detail::int_pow_impl(q.in(U{}), Exp));
}

//
// The value of the "smart" inverse of a Quantity, in a given destination Unit and Rep.
//
// This is the "explicit Rep" format, which is semantically equivalent to a `static_cast`.
//
template <typename TargetRep, typename TargetUnits, typename U, typename R>
constexpr auto inverse_in(TargetUnits target_units, Quantity<U, R> q) {
    using Rep = std::common_type_t<TargetRep, R>;
    constexpr auto UNITY = make_constant(UnitProductT<>{});
    return static_cast<TargetRep>(UNITY.in<Rep>(associated_unit(target_units) * U{}) / q.in(U{}));
}

//
// The value of the "smart" inverse of a Quantity, in a given destination unit.
//
// By "smart", we mean that, e.g., you can convert an integral Quantity of Kilo<Hertz> to an
// integral Quantity of Nano<Seconds>, without ever leaving the integral domain.  (Under the hood,
// in this case, the library will know to divide into 1'000'000 instead of dividing into 1.)
//
template <typename TargetUnits, typename U, typename R>
constexpr auto inverse_in(TargetUnits target_units, Quantity<U, R> q) {
    // The policy here is similar to our overflow policy, in that we try to avoid "bad outcomes"
    // when users store values less than 1000.  (The thinking, here as there, is that values _more_
    // than 1000 would tend to be stored in the next SI-prefixed unit up, e.g., 1 km instead of 1000
    // m.)
    //
    // The "bad outcome" here is a lossy conversion.  Since we're mainly worried about the integral
    // domain (because floating point numbers are already pretty well behaved), this means that:
    //
    //    inverse_in(a, inverse_as(b, a(n)))
    //
    // should be the identity for all n <= 1000.  For this to be true, we need a threshold of
    // (1'000 ^ 2) = 1'000'000.
    //
    // (An extreme instance of this kind of lossiness would be the inverse of a nonzero value
    // getting represented as 0, which would happen for values over the threshold.)

    // This will fail at compile time for types that can't hold 1'000'000.
    constexpr R threshold = 1'000'000;

    constexpr auto UNITY = make_constant(UnitProductT<>{});

    static_assert(
        UNITY.in<R>(associated_unit(target_units) * U{}) >= threshold ||
            std::is_floating_point<R>::value,
        "Dangerous inversion risking truncation to 0; must supply explicit Rep if truly desired");

    // Having passed safety checks (at compile time!), we can delegate to the explicit-Rep version.
    return inverse_in<R>(target_units, q);
}

//
// The "smart" inverse of a Quantity, in a given destination unit.
//
// (See `inverse_in()` comment above for how this inverse is "smart".)
//
template <typename TargetUnits, typename U, typename R>
constexpr auto inverse_as(TargetUnits target_units, Quantity<U, R> q) {
    return make_quantity<AssociatedUnitT<TargetUnits>>(inverse_in(target_units, q));
}

//
// The "smart" inverse of a Quantity, in a given destination Unit and Rep.
//
// This is the "explicit Rep" format, which is semantically equivalent to a `static_cast`.
//
template <typename TargetRep, typename TargetUnits, typename U, typename R>
constexpr auto inverse_as(TargetUnits target_units, Quantity<U, R> q) {
    return make_quantity<AssociatedUnitT<TargetUnits>>(inverse_in<TargetRep>(target_units, q));
}

//
// Check whether the value stored is "not a number" (NaN).
//
template <typename U, typename R>
constexpr bool isnan(Quantity<U, R> q) {
    return std::isnan(q.in(U{}));
}

// The maximum of two values of the same dimension.
//
// Unlike std::max, returns by value rather than by reference, because the types might differ.
template <typename U1, typename U2, typename R1, typename R2>
auto max(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    return detail::using_common_type(q1, q2, [](auto a, auto b) { return std::max(a, b); });
}

// Overload to resolve ambiguity with `std::max` for identical `Quantity` types.
template <typename U, typename R>
auto max(Quantity<U, R> a, Quantity<U, R> b) {
    return std::max(a, b);
}

// The maximum of two point values of the same dimension.
//
// Unlike std::max, returns by value rather than by reference, because the types might differ.
template <typename U1, typename U2, typename R1, typename R2>
auto max(QuantityPoint<U1, R1> p1, QuantityPoint<U2, R2> p2) {
    return detail::using_common_point_unit(p1, p2, [](auto a, auto b) { return std::max(a, b); });
}

// Overload to resolve ambiguity with `std::max` for identical `QuantityPoint` types.
template <typename U, typename R>
auto max(QuantityPoint<U, R> a, QuantityPoint<U, R> b) {
    return std::max(a, b);
}

// `max` overloads for when Zero is one of the arguments.
//
// NOTE: these will not work if _both_ arguments are `Zero`, but we don't plan to support this
// unless we find a compelling use case.
template <typename T>
auto max(Zero z, T x) {
    static_assert(std::is_convertible<Zero, T>::value,
                  "Cannot compare type to abstract notion Zero");
    return std::max(T{z}, x);
}
template <typename T>
auto max(T x, Zero z) {
    static_assert(std::is_convertible<Zero, T>::value,
                  "Cannot compare type to abstract notion Zero");
    return std::max(x, T{z});
}

// The minimum of two values of the same dimension.
//
// Unlike std::min, returns by value rather than by reference, because the types might differ.
template <typename U1, typename U2, typename R1, typename R2>
auto min(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    return detail::using_common_type(q1, q2, [](auto a, auto b) { return std::min(a, b); });
}

// Overload to resolve ambiguity with `std::min` for identical `Quantity` types.
template <typename U, typename R>
auto min(Quantity<U, R> a, Quantity<U, R> b) {
    return std::min(a, b);
}

// The minimum of two point values of the same dimension.
//
// Unlike std::min, returns by value rather than by reference, because the types might differ.
template <typename U1, typename U2, typename R1, typename R2>
auto min(QuantityPoint<U1, R1> p1, QuantityPoint<U2, R2> p2) {
    return detail::using_common_point_unit(p1, p2, [](auto a, auto b) { return std::min(a, b); });
}

// Overload to resolve ambiguity with `std::min` for identical `QuantityPoint` types.
template <typename U, typename R>
auto min(QuantityPoint<U, R> a, QuantityPoint<U, R> b) {
    return std::min(a, b);
}

// `min` overloads for when Zero is one of the arguments.
//
// NOTE: these will not work if _both_ arguments are `Zero`, but we don't plan to support this
// unless we find a compelling use case.
template <typename T>
auto min(Zero z, T x) {
    static_assert(std::is_convertible<Zero, T>::value,
                  "Cannot compare type to abstract notion Zero");
    return std::min(T{z}, x);
}
template <typename T>
auto min(T x, Zero z) {
    static_assert(std::is_convertible<Zero, T>::value,
                  "Cannot compare type to abstract notion Zero");
    return std::min(x, T{z});
}

// The (zero-centered) floating point remainder of two values of the same dimension.
template <typename U1, typename R1, typename U2, typename R2>
auto remainder(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    using U = CommonUnitT<U1, U2>;
    using R = decltype(std::remainder(R1{}, R2{}));
    return make_quantity<U>(std::remainder(q1.template in<R>(U{}), q2.template in<R>(U{})));
}

//
// Round the value of this Quantity to the nearest integer in the given units.
//
// This is the "Unit-only" format (i.e., `round_in(rounding_units, q)`).
//
template <typename RoundingUnits, typename U, typename R>
auto round_in(RoundingUnits rounding_units, Quantity<U, R> q) {
    using OurRoundingRep = detail::RoundingRepT<Quantity<U, R>, RoundingUnits>;
    return std::round(q.template in<OurRoundingRep>(rounding_units));
}

//
// Round the value of this Quantity to the nearest integer in the given units, returning OutputRep.
//
// This is the "Explicit-Rep" format (e.g., `round_in<int>(rounding_units, q)`).
//
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto round_in(RoundingUnits rounding_units, Quantity<U, R> q) {
    return static_cast<OutputRep>(round_in(rounding_units, q));
}

//
// The integral-valued Quantity, in this unit, nearest to the input.
//
// This is the "Unit-only" format (i.e., `round_as(rounding_units, q)`).
//
template <typename RoundingUnits, typename U, typename R>
auto round_as(RoundingUnits rounding_units, Quantity<U, R> q) {
    return make_quantity<AssociatedUnitT<RoundingUnits>>(round_in(rounding_units, q));
}

//
// The integral-valued Quantity, in this unit, nearest to the input, using the specified OutputRep.
//
// This is the "Explicit-Rep" format (e.g., `round_as<float>(rounding_units, q)`).
//
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto round_as(RoundingUnits rounding_units, Quantity<U, R> q) {
    return make_quantity<AssociatedUnitT<RoundingUnits>>(round_in<OutputRep>(rounding_units, q));
}

//
// Return the largest integral value in `rounding_units` which is not greater than `q`.
//
// This is the "Unit-only" format (i.e., `floor_in(rounding_units, q)`).
//
template <typename RoundingUnits, typename U, typename R>
auto floor_in(RoundingUnits rounding_units, Quantity<U, R> q) {
    using OurRoundingRep = detail::RoundingRepT<Quantity<U, R>, RoundingUnits>;
    return std::floor(q.template in<OurRoundingRep>(rounding_units));
}

//
// Return `OutputRep` with largest integral value in `rounding_units` which is not greater than `q`.
//
// This is the "Explicit-Rep" format (e.g., `floor_in<int>(rounding_units, q)`).
//
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto floor_in(RoundingUnits rounding_units, Quantity<U, R> q) {
    return static_cast<OutputRep>(floor_in(rounding_units, q));
}

//
// The largest integral-valued Quantity, in this unit, not greater than the input.
//
// This is the "Unit-only" format (i.e., `floor_as(rounding_units, q)`).
//
template <typename RoundingUnits, typename U, typename R>
auto floor_as(RoundingUnits rounding_units, Quantity<U, R> q) {
    return make_quantity<AssociatedUnitT<RoundingUnits>>(floor_in(rounding_units, q));
}

//
// The largest integral-valued Quantity, in this unit, not greater than the input, using the
// specified `OutputRep`.
//
// This is the "Explicit-Rep" format (e.g., `floor_as<float>(rounding_units, q)`).
//
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto floor_as(RoundingUnits rounding_units, Quantity<U, R> q) {
    return make_quantity<AssociatedUnitT<RoundingUnits>>(floor_in<OutputRep>(rounding_units, q));
}

//
// Return the smallest integral value in `rounding_units` which is not less than `q`.
//
// This is the "Unit-only" format (i.e., `ceil_in(rounding_units, q)`).
//
template <typename RoundingUnits, typename U, typename R>
auto ceil_in(RoundingUnits rounding_units, Quantity<U, R> q) {
    using OurRoundingRep = detail::RoundingRepT<Quantity<U, R>, RoundingUnits>;
    return std::ceil(q.template in<OurRoundingRep>(rounding_units));
}

//
// Return the smallest integral value in `rounding_units` which is not less than `q`.
//
// This is the "Explicit-Rep" format (e.g., `ceil_in<int>(rounding_units, q)`).
//
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto ceil_in(RoundingUnits rounding_units, Quantity<U, R> q) {
    return static_cast<OutputRep>(ceil_in(rounding_units, q));
}

//
// The smallest integral-valued Quantity, in this unit, not less than the input.
//
// This is the "Unit-only" format (i.e., `ceil_as(rounding_units, q)`).
//
template <typename RoundingUnits, typename U, typename R>
auto ceil_as(RoundingUnits rounding_units, Quantity<U, R> q) {
    return make_quantity<AssociatedUnitT<RoundingUnits>>(ceil_in(rounding_units, q));
}

//
// The smallest integral-valued Quantity, in this unit, not less than the input, using the specified
// `OutputRep`.
//
// This is the "Explicit-Rep" format (e.g., `ceil_as<float>(rounding_units, q)`).
//
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto ceil_as(RoundingUnits rounding_units, Quantity<U, R> q) {
    return make_quantity<AssociatedUnitT<RoundingUnits>>(ceil_in<OutputRep>(rounding_units, q));
}

// Wrapper for std::sin() which accepts a strongly typed angle quantity.
template <typename U, typename R>
auto sin(Quantity<U, R> q) {
    return std::sin(detail::in_radians(q));
}

// Wrapper for std::sqrt() which handles Quantity types.
template <typename U, typename R>
auto sqrt(Quantity<U, R> q) {
    return make_quantity<UnitPowerT<U, 1, 2>>(std::sqrt(q.in(U{})));
}

// Wrapper for std::tan() which accepts a strongly typed angle quantity.
template <typename U, typename R>
auto tan(Quantity<U, R> q) {
    return std::tan(detail::in_radians(q));
}

}  // namespace au

namespace std {
/// `numeric_limits` specialization.  The default implementation default constructs the scalar,
/// which would return the obviously-wrong value of 0 for max().
///
/// Per the standard, we are allowed to specialize this for our own types, and are also not required
/// to define every possible field. This is nice because it means that we will get compile errors
/// for unsupported operations (instead of having them silently fail, which is the default)
///
/// Source: https://stackoverflow.com/a/16519653
template <typename U, typename R>
struct numeric_limits<au::Quantity<U, R>> {
    // To validily extent std::numeric_limits<T>, we must define all members declared static
    // constexpr in the primary template, in such a way that they are usable as integral constant
    // expressions.
    //
    // Source for rule: https://en.cppreference.com/w/cpp/language/extending_std
    // List of members: https://en.cppreference.com/w/cpp/types/numeric_limits
    static constexpr bool is_specialized = true;
    static constexpr bool is_integer = numeric_limits<R>::is_integer;
    static constexpr bool is_signed = numeric_limits<R>::is_signed;
    static constexpr bool is_exact = numeric_limits<R>::is_exact;
    static constexpr bool has_infinity = numeric_limits<R>::has_infinity;
    static constexpr bool has_quiet_NaN = numeric_limits<R>::has_quiet_NaN;
    static constexpr bool has_signaling_NaN = numeric_limits<R>::has_signaling_NaN;
    static constexpr bool has_denorm = numeric_limits<R>::has_denorm;
    static constexpr bool has_denorm_loss = numeric_limits<R>::has_denorm_loss;
    static constexpr float_round_style round_style = numeric_limits<R>::round_style;
    static constexpr bool is_iec559 = numeric_limits<R>::is_iec559;
    static constexpr bool is_bounded = numeric_limits<R>::is_bounded;
    static constexpr bool is_modulo = numeric_limits<R>::is_modulo;
    static constexpr int digits = numeric_limits<R>::digits;
    static constexpr int digits10 = numeric_limits<R>::digits10;
    static constexpr int max_digits10 = numeric_limits<R>::max_digits10;
    static constexpr int radix = numeric_limits<R>::radix;
    static constexpr int min_exponent = numeric_limits<R>::min_exponent;
    static constexpr int min_exponent10 = numeric_limits<R>::min_exponent10;
    static constexpr int max_exponent = numeric_limits<R>::max_exponent;
    static constexpr int max_exponent10 = numeric_limits<R>::max_exponent10;
    static constexpr bool traps = numeric_limits<R>::traps;
    static constexpr bool tinyness_before = numeric_limits<R>::tinyness_before;

    static constexpr au::Quantity<U, R> max() {
        return au::make_quantity<U>(std::numeric_limits<R>::max());
    }

    static constexpr au::Quantity<U, R> lowest() {
        return au::make_quantity<U>(std::numeric_limits<R>::lowest());
    }

    static constexpr au::Quantity<U, R> min() {
        return au::make_quantity<U>(std::numeric_limits<R>::min());
    }

    static constexpr au::Quantity<U, R> epsilon() {
        return au::make_quantity<U>(std::numeric_limits<R>::epsilon());
    }

    static constexpr au::Quantity<U, R> round_error() {
        return au::make_quantity<U>(std::numeric_limits<R>::round_error());
    }

    static constexpr au::Quantity<U, R> infinity() {
        return au::make_quantity<U>(std::numeric_limits<R>::infinity());
    }

    static constexpr au::Quantity<U, R> quiet_NaN() {
        return au::make_quantity<U>(std::numeric_limits<R>::quiet_NaN());
    }

    static constexpr au::Quantity<U, R> signaling_NaN() {
        return au::make_quantity<U>(std::numeric_limits<R>::signaling_NaN());
    }

    static constexpr au::Quantity<U, R> denorm_min() {
        return au::make_quantity<U>(std::numeric_limits<R>::denorm_min());
    }
};

// Specialize for cv-qualified Quantity types by inheriting from bare Quantity implementation.
template <typename U, typename R>
struct numeric_limits<const au::Quantity<U, R>> : numeric_limits<au::Quantity<U, R>> {};
template <typename U, typename R>
struct numeric_limits<volatile au::Quantity<U, R>> : numeric_limits<au::Quantity<U, R>> {};
template <typename U, typename R>
struct numeric_limits<const volatile au::Quantity<U, R>> : numeric_limits<au::Quantity<U, R>> {};

template <typename U, typename R>
constexpr bool numeric_limits<au::Quantity<U, R>>::is_specialized;

template <typename U, typename R>
constexpr bool numeric_limits<au::Quantity<U, R>>::is_integer;

template <typename U, typename R>
constexpr bool numeric_limits<au::Quantity<U, R>>::is_signed;

template <typename U, typename R>
constexpr bool numeric_limits<au::Quantity<U, R>>::is_exact;

template <typename U, typename R>
constexpr bool numeric_limits<au::Quantity<U, R>>::has_infinity;

template <typename U, typename R>
constexpr bool numeric_limits<au::Quantity<U, R>>::has_quiet_NaN;

template <typename U, typename R>
constexpr bool numeric_limits<au::Quantity<U, R>>::has_signaling_NaN;

template <typename U, typename R>
constexpr bool numeric_limits<au::Quantity<U, R>>::has_denorm;

template <typename U, typename R>
constexpr bool numeric_limits<au::Quantity<U, R>>::has_denorm_loss;

template <typename U, typename R>
constexpr float_round_style numeric_limits<au::Quantity<U, R>>::round_style;

template <typename U, typename R>
constexpr bool numeric_limits<au::Quantity<U, R>>::is_iec559;

template <typename U, typename R>
constexpr bool numeric_limits<au::Quantity<U, R>>::is_bounded;

template <typename U, typename R>
constexpr bool numeric_limits<au::Quantity<U, R>>::is_modulo;

template <typename U, typename R>
constexpr int numeric_limits<au::Quantity<U, R>>::digits;

template <typename U, typename R>
constexpr int numeric_limits<au::Quantity<U, R>>::digits10;

template <typename U, typename R>
constexpr int numeric_limits<au::Quantity<U, R>>::max_digits10;

template <typename U, typename R>
constexpr int numeric_limits<au::Quantity<U, R>>::radix;

template <typename U, typename R>
constexpr int numeric_limits<au::Quantity<U, R>>::min_exponent;

template <typename U, typename R>
constexpr int numeric_limits<au::Quantity<U, R>>::min_exponent10;

template <typename U, typename R>
constexpr int numeric_limits<au::Quantity<U, R>>::max_exponent;

template <typename U, typename R>
constexpr int numeric_limits<au::Quantity<U, R>>::max_exponent10;

template <typename U, typename R>
constexpr bool numeric_limits<au::Quantity<U, R>>::traps;

template <typename U, typename R>
constexpr bool numeric_limits<au::Quantity<U, R>>::tinyness_before;

}  // namespace std

