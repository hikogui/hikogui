//
//  Time.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-12.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#ifdef _WIN32
#include <Windows.h>
#endif
#include <time.h>
#include <boost/multiprecision/cpp_int.hpp>
#include <cstdint>

namespace TTauri {

/*! Timestamp
 * The timestamp is defined as the number of nanoseconds since 1970-01-01 00:00:00.000000000 on
 * the TAI time standard. This is the same format for PTP timestamps which is also the recommendation
 * for processing time for audio/video by SMPTE.
 */
class Timestamp {
public:
    int64_t intrinsic;

#ifdef _WIN32
	/*! Get a timestamp based on a high resolution system clock.
     */
	static Timestamp now(void) {
		FILETIME ts; 

		GetSystemTimePreciseAsFileTime(&ts);

		auto utc_ts = static_cast<int64_t>(ts.dwHighDateTime) << 32;
		utc_ts |= static_cast<int64_t>(ts.dwLowDateTime);

		// Convert to UNIX Epoch. Currently utc_ts is still in 100ns format.
		// Convert  1601-01-1 00:00:00 -> 1970-01-01 00:00:00
		utc_ts -= 116444736000000000;

		// Convert to 1ns format.
		utc_ts *= 100;

		auto ptp_ts = utc_ts;
		return { ptp_ts };
	}

#else
    /*! Get a timestamp based on a high resolution system clock.
     */
    static Timestamp now(void) {
        struct timespec ts;

        // This should never return an error, but it needs to be fast too.
        clock_gettime(CLOCK_REALTIME, &ts);

        auto utc_ts = static_cast<int64_t>(ts.tv_sec) * 1000000;
        utc_ts += ts.tv_nsec;

        auto ptp_ts = utc_ts;
        return {ptp_ts};
    }
#endif
};

struct ClockCalibration {
    uint64_t gain;
    uint64_t bias;
};

/*! A clock is an automatically calibrating clock.
 */
class Clock {
public:
    Clock();
    ~Clock();

    ClockCalibration *calibration;
    ClockCalibration calibrations[2];

    /*! Number of leap second detected.
     * During calibration leapSeconds are added to the absoluteTime
     * so as not to change the calibration during a skip or double second time.
     */
    int64_t leapSeconds;

    /*! Return a timestamp from a clock.
     */
    Timestamp operator()(uint64_t counter) const {
        volatile auto c = calibration;

        auto counter128 = static_cast<boost::multiprecision::int128_t>(counter);
        counter128 *= c->gain;
        counter128 >>= 64;
        counter128 += c->bias;
        return {static_cast<int64_t>(counter128)};
    }

    /*! Calibrate the clock by comparing a counter with absolute time.
     */
    Timestamp calibrate(uint64_t counter, Timestamp absoluteTime);

    /*! Calibrate the clock by comparing a counter with the current system time.
     */
    Timestamp calibrate(uint64_t counter) {
        auto absoluteTime = Timestamp::now();
        return calibrate(counter, absoluteTime);
    }
};

}
