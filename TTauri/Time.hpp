// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include "logging.hpp"

#ifdef _WIN32
#include <Windows.h>
#include <intrin.h>
#endif
#include <time.h>
#include <boost/multiprecision/cpp_int.hpp>
#include <cstdint>
#include <chrono>
#include <ratio>
#include <atomic>

namespace TTauri {

struct rdtsc_clock {
    using rep = uint64_t;
    // Technically not nano-seconds, this just a counter.
    using period = std::nano;
    using duration = std::chrono::duration<rep, period>;
    using timepoint = std::chrono::time_point<rdtsc_clock>;
    static const bool is_steady = true;

#ifdef _WIN32
    /*! Get a timestamp based on a high resolution system clock.
    */
    static timepoint now() {
        return timepoint(duration(__rdtsc()));
    }
#endif
};

/*! Timestamp
 * The timestamp is defined as the number of nanoseconds since 1970-01-01 00:00:00.000000000 on
 * the TAI time standard. This is the same format for PTP timestamps which is also the recommendation
 * for processing time for audio/video by SMPTE.
 */
struct tai_system_clock {
    using rep = int64_t;
    using period = std::nano;
    using duration = std::chrono::duration<rep, period>;
    using timepoint = std::chrono::time_point<tai_system_clock>;
    static const bool is_steady = false;

#ifdef _WIN32
	/*! Get a timestamp based on a high resolution system clock.
     */
	static timepoint now() {
		FILETIME ts; 

		GetSystemTimePreciseAsFileTime(&ts);

		auto utc_ts = static_cast<int64_t>(ts.dwHighDateTime) << 32;
		utc_ts |= static_cast<int64_t>(ts.dwLowDateTime);

		// Convert to UNIX Epoch. Currently utc_ts is still in 100ns format.
		// Convert  1601-01-1 00:00:00 -> 1970-01-01 00:00:00
		utc_ts -= 116444736000000000;

		// Convert to 1ns format.
		utc_ts *= 100;

        // XXX Correctly set convert to TAI.
		let ptp_ts = utc_ts;
		return timepoint(duration(ptp_ts));
	}

#else
    /*! Get a timestamp based on a high resolution system clock.
     */
    static timepoint now() {
        struct timespec ts;

        // This should never return an error, but it needs to be fast too.
        clock_gettime(CLOCK_REALTIME, &ts);

        auto utc_ts = static_cast<int64_t>(ts.tv_sec) * 1000000;
        utc_ts += ts.tv_nsec;

        // XXX Correctly set convert to TAI.
        auto ptp_ts = utc_ts;
        return timepoint(duration(ptp_ts));
    }
#endif
};


/*! A clock is an automatically calibrating clock.
 */
template<typename C1, typename C2> 
struct clock_sync {
    using slow_clock = C1;
    using fast_clock = C2;

    using rep = typename slow_clock::rep;
    using period = typename slow_clock::period;
    using duration = typename slow_clock::duration;
    using timepoint = typename slow_clock::timepoint;
    static const bool is_steady = slow_clock::is_steady;

    struct calibration_t {
        static constexpr int gainShift = 60;
        static constexpr double gainMultiplier = static_cast<double>(1ULL << gainShift);

        int64_t gain = 0;
        int64_t bias = 0;
    };

    static inline std::atomic<calibration_t> calibration;

    static inline std::pair<typename slow_clock::timepoint, typename fast_clock::timepoint> previousTimepoints;

    static constexpr size_t maxNrGains = 20;
    static inline size_t gainCount = 0;
    static inline std::array<double, maxNrGains> gains;

    static void calibrate() {
        let now_slow = slow_clock::now();
        let now_fast = fast_clock::now();

        let [prev_slow, prev_fast] = previousTimepoints;
        previousTimepoints = {now_slow, now_fast};

        let hasPreviousTimepoints = prev_slow.time_since_epoch().count() != 0;

        if (hasPreviousTimepoints) {
            // Calculate a new gain, compared to the previous timepoints.
            let diff_slow = now_slow - prev_slow;
            let diff_fast = now_fast - prev_fast;
            let gain = static_cast<double>(diff_slow.count()) / static_cast<double>(diff_fast.count());

            gains[gainCount++ % maxNrGains] = gain;
        }

        // Prepare the inter-quartiel-range.
        let gainCountClamped = std::min(gainCount, maxNrGains);
        auto sortedGains = gains;
        std::sort(sortedGains.begin(), sortedGains.begin() + gainCountClamped);
        let iqr_begin = sortedGains.begin() + (gainCountClamped / 4);
        let iqr_end = sortedGains.begin() + ((gainCountClamped * 3) / 4);
        let iqr_length = iqr_end - iqr_begin;

        if (iqr_length > 0) {
            // Calculate the arithmatic mean over the inter-quartiel-range
            let mean_gain = std::accumulate(iqr_begin, iqr_end, 0.0) / static_cast<double>(iqr_length);

            // Calculate the new calibration values.
            let now_gain = static_cast<int64_t>(mean_gain * calibration_t::gainMultiplier + 0.5);
            auto u128_count = static_cast<boost::multiprecision::int128_t>(now_fast.time_since_epoch().count());
            u128_count *= now_gain;
            u128_count >>= calibration_t::gainShift;
            let now_bias = now_slow.time_since_epoch().count() - static_cast<int64_t>(u128_count);

            calibration.store({now_gain, now_bias});
        }
    }

    static duration checkCalibration() {
        let now_slow = slow_clock::now();
        let now_fast = fast_clock::now();
        let now_fast_as_slow = convert(now_fast);
        return now_fast_as_slow - now_slow;
    }

    /*! Return a timestamp from a clock.
     */
    static timepoint convert(typename fast_clock::timepoint fastTime) {
        let c = calibration.load();

        auto u128_count = static_cast<boost::multiprecision::int128_t>(fastTime.time_since_epoch().count());
        u128_count *= c.gain;
        u128_count >>= calibration_t::gainShift;
        u128_count += c.bias;

        let i64_count = static_cast<typename rep>(u128_count);
        let slow_period = duration(i64_count);
        let slow_timepoint = timepoint(slow_period);
        return slow_timepoint;
    }

    static timepoint now() {
        return clock_sync::convert(fast_clock::now());
    }
};

}
