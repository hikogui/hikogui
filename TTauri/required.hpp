// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <boost/numeric/conversion/cast.hpp>
#include <exception>

/*! Invariant should be the default for variables.
 * C++ does have an invariant but it requires you to enter the 'const' keyword which
 * is easy to forget. Using a single keyword 'let' for an invariant makes it easier to notice
 * when you have defined a variant.
 */
#define let auto const

#define required_assert(x) if (!(x)) { std::terminate(); }

#define no_default std::terminate()

namespace TTauri {
template<typename T, typename U>
[[gsl::suppress(f.6)]]
T numeric_cast(U value) noexcept {
    return boost::numeric_cast<T>(value);
}
}

#define to_int(x) TTauri::numeric_cast<int>(x)
#define to_int64(x) TTauri::numeric_cast<int64_t>(x)

#if defined(_MSC_VER)
#define gsl_suppress(a) [[gsl::suppress(a)]]
#define gsl_suppress2(a,b) [[gsl::suppress(a)]] [[gsl::suppress(b)]]
#define gsl_suppress3(a,b,c) [[gsl::suppress(a)]] [[gsl::suppress(b)]] [[gsl::suppress(c)]]
#define gsl_suppress4(a,b,c,d) [[gsl::suppress(a)]] [[gsl::suppress(b)]] [[gsl::suppress(c)]] [[gsl::suppress(d)]]
#define gsl_suppress5(a,b,c,d,e) [[gsl::suppress(a)]] [[gsl::suppress(b)]] [[gsl::suppress(c)]] [[gsl::suppress(d)]] [[gsl::suppress(e)]]
#elif defined(__clang__)
#define gsl_suppress(a) [[gsl::suppress(#a)]]
#define gsl_suppress2(a,b) [[gsl::suppress(#a)]] [[gsl::suppress(#b)]]
#define gsl_suppress3(a,b,c) [[gsl::suppress(#a)]] [[gsl::suppress(#b)]] [[gsl::suppress(#c)]]
#define gsl_suppress4(a,b,c,d) [[gsl::suppress(#a)]] [[gsl::suppress(#b)]] [[gsl::suppress(#c)]] [[gsl::suppress(#d)]]
#define gsl_suppress5(a,b,c,d,e) [[gsl::suppress(#a)]] [[gsl::suppress(#b)]] [[gsl::suppress(#c)]] [[gsl::suppress(#d)]] [[gsl::suppress(#e)]]
#else
#define gsl_suppress(a)
#define gsl_suppress2(a,b)
#define gsl_suppress3(a,b,c)
#define gsl_suppress4(a,b,c,d)
#define gsl_suppress5(a,b,c,d,e)
#endif
