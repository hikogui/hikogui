// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include "thread.hpp"
#include "bigint.hpp"
#include <fmt/format.h>
#include <chrono>
#include <algorithm>
#include <string>
#include <vector>
#include <optional>
#include <atomic>

using namespace std::literals::chrono_literals;

namespace tt {

template<typename C1, typename C2>
class sync_clock_calibration_type {
    using slow_clock = C1;
    using fast_clock = C2;

    struct time_point_pair {
        typename slow_clock::time_point slow;
        typename fast_clock::time_point fast;
    };

    time_point_pair first_pair;
    time_point_pair prev_pair;
    time_point_pair last_pair;
    int calibration_nr = 0;

    static constexpr int gainShift = 60;
    static constexpr double gainMultiplier = static_cast<double>(1ULL << gainShift);

    std::atomic<int64_t> gain = 0;
    std::atomic<typename slow_clock::duration> bias = 0ns;

    /*! When during calibration we detect a leap second, we will update this offset (in ns).
    */
    typename slow_clock::duration leapsecond_offset = 0ns;

    std::string name;

public:
    /** Construct a sync clock.
     * @param name The name of the calibrated clock.
     */
    sync_clock_calibration_type(std::string name) noexcept :
        name(std::move(name))
    {
        // Do a first calibration of the clock.
        // Second calibration is done by the calibrate_loop thread.
        calibrate();
    }

    ~sync_clock_calibration_type() {
    }

    typename slow_clock::time_point convert(typename fast_clock::time_point fast_time) const noexcept {
        return convert(gain.load(std::memory_order_relaxed), bias.load(std::memory_order::relaxed), fast_time);
    }

    typename slow_clock::duration convert(typename fast_clock::duration fast_duration) const noexcept {
        return convert(gain.load(std::memory_order::relaxed), fast_duration);
    }

    /* Calibrate the sync clock.
    * Should be called from the maintenance thread every 100ms.
    */
    void calibrate_tick() noexcept {
        auto backoff = 0s;

        if (calibration_nr > 2) {
            backoff = (calibration_nr - 2) * 10s;
        }

        if (backoff > 120s) {
            backoff = 120s;
        }

        if (last_pair.slow + backoff < slow_clock::now()) {
            calibrate();
        }
    }

private:
    static time_point_pair makeCalibrationPoint() noexcept {
        // We are going to read the slow clock twice sandwiched by fast clocks,
        // we expect that it will not be interrupted by a time-slice more than once.
        ttlet f1 = fast_clock::now();
        ttlet s1 = slow_clock::now();
        ttlet f2 = fast_clock::now();
        ttlet s2 = slow_clock::now();
        ttlet f3 = fast_clock::now();

        if ((f2 - f1) < (f3 - f2)) {
            return {s1, f1};
        } else {
            return {s2, f2};
        }
    }

    void addCalibrationPoint() noexcept {
        auto tp = makeCalibrationPoint();
        if (calibration_nr++ == 0) {
            first_pair = tp;
        }
        prev_pair = last_pair;
        last_pair = tp;
    }

    int64_t getGain() noexcept {
        // Calculate the gain between the current and first calibration.
        ttlet diff_slow = static_cast<double>((last_pair.slow - first_pair.slow).count());
        ttlet diff_fast = static_cast<double>((last_pair.fast - first_pair.fast).count());
    
        if (calibration_nr < 2 || diff_fast == 0.0) {
            return static_cast<int64_t>(gainMultiplier);
        } else {
            auto new_gain = diff_slow / diff_fast;
            return static_cast<int64_t>(std::round(new_gain * gainMultiplier));
        }
    }

    typename slow_clock::duration getBias(int64_t new_gain) noexcept {
        // Get a large integer cpu_clock_value.
        auto tmp = static_cast<ubig128>(last_pair.fast.time_since_epoch() / 1ns);

        // Multiply with the integer gain, that is pre-multiplied.
        tmp *= new_gain;

        // Add half of the lost precision for proper rounding.
        tmp += (1LL << (gainShift - 1));

        // Remove gain-pre-multiplier.
        tmp >>= gainShift;

        ttlet now_fast_after_gain = typename slow_clock::duration(static_cast<typename slow_clock::rep>(tmp));

        return (last_pair.slow.time_since_epoch() + leapsecond_offset) - now_fast_after_gain;
    }

    typename slow_clock::duration getLeapAdjustment(int64_t new_gain, typename slow_clock::duration new_bias)
    {
        // Check and update for leap second.
        ttlet prev_fast_as_slow = convert(last_pair.fast);
        ttlet next_fast_as_slow = convert(new_gain, new_bias, last_pair.fast);
        ttlet diff_fast_as_slow = prev_fast_as_slow - next_fast_as_slow;

        return
            (diff_fast_as_slow >= 999ms && diff_fast_as_slow <= 1001ms) ?
            -1s :
            (diff_fast_as_slow >= -1001ms && diff_fast_as_slow <= -999ms) ?
            +1s :
            0s;
    }

    /*! Return the amount of drift from fast to slow clock, since last calibration.
     * This function must be called before the new gain and bias are set.
     */
    double getDrift() noexcept {
        // Compare the new calibration point, with the old calibration data.
        ttlet fast_to_slow_offset = convert(last_pair.fast) - last_pair.slow;
        ttlet fast_to_slow_offset_ns = static_cast<double>(fast_to_slow_offset / 1ns);

        ttlet duration_since_calibration = last_pair.slow - prev_pair.slow;
        ttlet duration_since_calibration_ns = static_cast<double>(duration_since_calibration / 1ns);
        return fast_to_slow_offset_ns / duration_since_calibration_ns;
    }

    void calibrate() noexcept {
        addCalibrationPoint();

        ttlet drift = getDrift();

        ttlet do_gain_calibration = calibration_nr <= 5;

        ttlet new_gain = do_gain_calibration ? getGain() : gain.load(std::memory_order::relaxed);
        ttlet new_bias = getBias(new_gain);
        ttlet leap_adjustment = getLeapAdjustment(new_gain, new_bias);

        // XXX implement leap second testing.
        if (leap_adjustment != 0ns) {
            tt_log_info("Clock '{}' detected leap-second {} s", name, leap_adjustment / 1s);
        }

        if (do_gain_calibration) {
            tt_log_info("Clock '{}' calibration {}: drift={:+} ns/s gain={:+.15} ns/tick",
                name, calibration_nr, drift * 1000000000.0, new_gain / gainMultiplier
            );
        } else {
            tt_log_info("Clock '{}' calibration {}: drift={:+} ns/s",
                name, calibration_nr, drift * 1000000000.0
            );
        }
        
        if (do_gain_calibration) {
            gain.store(new_gain, std::memory_order::relaxed);
        }
        bias.store(new_bias + leap_adjustment, std::memory_order::relaxed);
        leapsecond_offset += leap_adjustment;
    }

    typename slow_clock::duration convert(int64_t new_gain, typename fast_clock::duration fast_duration) const noexcept {
        ttlet _new_gain = static_cast<uint64_t>(new_gain);
        ttlet _fast_duration = static_cast<uint64_t>(fast_duration.count());

        ttlet [lo, hi] = wide_multiply(_new_gain, _fast_duration);

        static_assert(gainShift < 64);
        ttlet slow_duration = (lo >> gainShift) | (hi << (64 - gainShift));

        return typename slow_clock::duration(static_cast<typename slow_clock::rep>(slow_duration));
    }

    typename slow_clock::time_point convert(int64_t new_gain, typename slow_clock::duration new_bias, typename fast_clock::time_point fast_time) const noexcept {
        ttlet slow_period = convert(new_gain, fast_time.time_since_epoch());
        return typename slow_clock::time_point(slow_period) + new_bias;
    }
};

template<typename C1, typename C2>
inline sync_clock_calibration_type<C1,C2> *sync_clock_calibration = nullptr;

/*! A clock which converts one clock to another clock.
 * The new clock is similar to C1 (slow clock), except that leap seconds from C1 are filtered out.
 * Leap seconds are filtered out because calibration to the slow clock does not happen often
 * enough to react in-time to a leap second.
 *
 * This clock is most often used to convert a cpu_counter_clock to a hires_tai_clock.
 *
 * \param C1 A clock with known epoch and known frequency.
 * \param C2 A monotonic clock which may have an unknown epoch and/or unknown frequency.
 */
template<typename C1, typename C2> 
struct sync_clock {
    using slow_clock = C1;
    using fast_clock = C2;

    using rep = typename slow_clock::rep;
    using period = typename slow_clock::period;
    using duration = typename slow_clock::duration;
    using time_point = typename slow_clock::time_point;
    static const bool is_steady = slow_clock::is_steady;

    /*! Return a timestamp from a clock.
     */
    static time_point convert(typename fast_clock::time_point fast_time) noexcept {
        return sync_clock_calibration<slow_clock,fast_clock> != nullptr ?
            sync_clock_calibration<slow_clock,fast_clock>->convert(fast_time) :
            time_point(duration(0ns));
    }

    static duration convert(typename fast_clock::duration fast_duration) noexcept {
        return sync_clock_calibration<slow_clock,fast_clock> != nullptr ?
            sync_clock_calibration<slow_clock,fast_clock>->convert(fast_duration) :
            duration(0ns);
    }


    static time_point now() noexcept {
        return convert(fast_clock::now());
    }
};

}
