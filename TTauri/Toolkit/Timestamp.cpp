//
//  Timestamp.cpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-13.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#include "Timestamp.hpp"
#include <time.h>

namespace TTauri {
namespace Toolkit {

#ifdef CLOCK_REALTIME
static int64_t utc_now(void)
{
    struct timespec ts;

    // This should never return an error, but it needs to be fast too.
    clock_gettime(CLOCK_REALTIME, &ts);

    auto utc_ts = static_cast<int64_t>(ts.tv_sec) * 1000000;
    utc_ts += ts.tv_nsec;

    return utc_ts;
}
#else
static int64_t utc_now(void)
{
    int64_t ts;

    GetSystemTimeAsFileTime(FILETIME *)&ts);
    ts -= 116444736000000000; // 1601-01-01 -> 1970-01-01
    ts *= 100; // 100ns -> 1ns;

    return ts;
}
#endif

#ifdef CLOCK_TAI
// Need to check if adjtimex(timex tmx) shows a TAI offset.
static int64_t tai_now(void)
{
    struct timespec ts;

    // This should never return an error, but it needs to be fast too.
    clock_gettime(CLOCK_TAI, &ts);

    auto utc_ts = static_cast<int64_t>(ts.tv_sec) * 1000000;
    utc_ts += ts.tv_nsec;

    return utc_ts;
}
#else

#endif


Timestamp Timestamp::now(void) {
    return {utc_now()};
}

}}
