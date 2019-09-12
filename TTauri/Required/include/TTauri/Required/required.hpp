// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "os_detect.hpp"
#include <exception>

/*! Invariant should be the default for variables.
 * C++ does have an invariant but it requires you to enter the 'const' keyword which
 * is easy to forget. Using a single keyword 'let' for an invariant makes it easier to notice
 * when you have defined a variant.
 */
#define let auto const

/*! required assert will always abort and can not be disabled in production code.
 */
#define required_assert(x) if (!(x)) { std::terminate(); }

#define no_default std::terminate()
#define not_implemented std::terminate()

#define to_int(x) TTauri::numeric_cast<int>(x)
#define to_int64(x) TTauri::numeric_cast<int64_t>(x)

#define STRINGIFY(a) #a

#if COMPILER == CC_MSVC
#define ttauri_likely(condition) condition
#define ttauri_unlikely(condition) condition
#define force_inline __forceinline
#define no_inline __declspec(noinline)
#define force_inline_attr
#define no_inline_attr
#define clang_suppress(a)
#define gsl_suppress(a) [[gsl::suppress(a)]]
#define gsl_suppress2(a,b) [[gsl::suppress(a)]] [[gsl::suppress(b)]]
#define gsl_suppress3(a,b,c) [[gsl::suppress(a)]] [[gsl::suppress(b)]] [[gsl::suppress(c)]]
#define gsl_suppress4(a,b,c,d) [[gsl::suppress(a)]] [[gsl::suppress(b)]] [[gsl::suppress(c)]] [[gsl::suppress(d)]]
#define gsl_suppress5(a,b,c,d,e) [[gsl::suppress(a)]] [[gsl::suppress(b)]] [[gsl::suppress(c)]] [[gsl::suppress(d)]] [[gsl::suppress(e)]]

#elif COMPILER == CC_CLANG
#define ttauri_likely(condition) __builtin_expect(static_cast<bool>(condition), 1)
#define ttauri_unlikely(condition) __builtin_expect(static_cast<bool>(condition), 0)
#define force_inline inline __attribute__((always_inline))
#define no_inline inline __attribute__((noinline))
#define force_inline_attr
#define no_inline_attr
#define clang_suppress(a) _Pragma(STRINGIFY(clang diagnostic ignored a))
#define gsl_suppress(a) [[gsl::suppress(#a)]]
#define gsl_suppress2(a,b) [[gsl::suppress(#a)]] [[gsl::suppress(#b)]]
#define gsl_suppress3(a,b,c) [[gsl::suppress(#a)]] [[gsl::suppress(#b)]] [[gsl::suppress(#c)]]
#define gsl_suppress4(a,b,c,d) [[gsl::suppress(#a)]] [[gsl::suppress(#b)]] [[gsl::suppress(#c)]] [[gsl::suppress(#d)]]
#define gsl_suppress5(a,b,c,d,e) [[gsl::suppress(#a)]] [[gsl::suppress(#b)]] [[gsl::suppress(#c)]] [[gsl::suppress(#d)]] [[gsl::suppress(#e)]]

#elif COMPILER == CC_GCC
#define ttauri_likely(condition) __builtin_expect(static_cast<bool>(condition), 1)
#define ttauri_unlikely(condition) __builtin_expect(static_cast<bool>(condition), 0)
#define force_inline inline __attribute__((always_inline))
#define no_inline inline __attribute__((noinline))
#define force_inline_attr
#define no_inline_attr
#define clang_suppress(a)
#define gsl_suppress(a) [[gsl::suppress(#a)]]
#define gsl_suppress2(a,b) [[gsl::suppress(#a)]] [[gsl::suppress(#b)]]
#define gsl_suppress3(a,b,c) [[gsl::suppress(#a)]] [[gsl::suppress(#b)]] [[gsl::suppress(#c)]]
#define gsl_suppress4(a,b,c,d) [[gsl::suppress(#a)]] [[gsl::suppress(#b)]] [[gsl::suppress(#c)]] [[gsl::suppress(#d)]]
#define gsl_suppress5(a,b,c,d,e) [[gsl::suppress(#a)]] [[gsl::suppress(#b)]] [[gsl::suppress(#c)]] [[gsl::suppress(#d)]] [[gsl::suppress(#e)]]

#else
#define ttauri_likely(condition) condition
#define ttauri_unlikely(condition) condition
#define force_inline inline
#define clang_suppress(a)
#define gsl_suppress(a)
#define gsl_suppress2(a,b)
#define gsl_suppress3(a,b,c)
#define gsl_suppress4(a,b,c,d)
#define gsl_suppress5(a,b,c,d,e)

#endif

constexpr size_t cache_line_size = 128;

namespace TTauri {
template<typename T, typename U>
gsl_suppress(f.6)
T numeric_cast(U value) noexcept {
    return static_cast<T>(value);
}
}

