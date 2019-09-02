// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "logger.hpp"
#include "counters.hpp"
#include <boost/multiprecision/cpp_int.hpp>
#include <chrono>
#include <algorithm>

using namespace std::literals::chrono_literals;

namespace TTauri {


template<typename C1, typename C2>
class sync_clock_calibration_type {
    using slow_clock = C1;
    using fast_clock = C2;

    struct time_point_pair {
        typename slow_clock::time_point slow;
        typename fast_clock::time_point fast;
    };

    time_point_pair first_pair;
    time_point_pair last_pair;
    int nr_calibrations = 0;

    static constexpr int gainShift = 60;
    static constexpr double gainMultiplier = static_cast<double>(1ULL << gainShift);

    std::atomic<int64_t> gain = 0;
    std::atomic<typename slow_clock::duration> bias = 0ns;

    /*! When during calibration we detect a leap second, we will update this offset (in ns).
    */
    typename slow_clock::duration leapsecond_offset = 0ns;

    std::thread calibrate_loop_id;
    bool calibrate_loop_stop = false;
    size_t calibrate_loop_count = 0;

public:
    /*! Construct a sync clock.
     * \param create_thread can be set to false when testing.
     */
    sync_clock_calibration_type(bool create_thread=true) noexcept {
        calibrate(slow_clock::now(), fast_clock::now());
        calibrate(slow_clock::now(), fast_clock::now());

        if (create_thread) {
            calibrate_loop_id = std::thread([&]() {
                return this->calibrate_loop();
            });
        }
    }

    ~sync_clock_calibration_type() {
        calibrate_loop_stop = true;
        if (calibrate_loop_id.joinable()) {
            calibrate_loop_id.join();
        }
    }

    typename slow_clock::time_point convert(typename fast_clock::time_point fast_time) const noexcept {
        return convert(gain.load(std::memory_order_relaxed), bias.load(std::memory_order_relaxed), fast_time);
    }

private:
    static time_point_pair makeCalibrationPoint() noexcept {
        // We are going to read the slow clock twice sandwiched by fast clocks,
        // we expect that it will not be interupted by a time-slice more than once.
        let f1 = fast_clock::now();
        let s1 = slow_clock::now();
        let f2 = fast_clock::now();
        let s2 = slow_clock::now();
        let f3 = fast_clock::now();

        if ((f2 - f1) < (f3 - f2)) {
            return {s1, f1};
        } else {
            return {s2, f2};
        }
    }

    void addCalibrationPoint() noexcept {
        auto tp = makeCalibrationPoint();
        if (nr_calibrations++ == 0) {
            first_pair = tp;
        }
        last_pair = tp;
    }

    int64_t getGain() noexcept {
        // Calculate the gain between the current and first calibration.
        let diff_slow = static_cast<double>((last_pair.slow - first_pair.slow).count());
        let diff_fast = static_cast<double>((last_pair.fast - first_pair.fast).count());
    
        if (nr_calibrations < 2 || diff_fast == 0.0) {
            return static_cast<int64_t>(gainMultiplier);
        } else {
            auto new_gain = diff_slow / diff_fast;
            return static_cast<int64_t>(new_gain * gainMultiplier);
        }
    }

    typename slow_clock::duration getBias(int64_t new_gain) noexcept {
        // Calculate the new calibration values.
        let now_fast_after_gain = typename slow_clock::duration(static_cast<int64_t>((
            static_cast<boost::multiprecision::int128_t>(last_pair.fast.time_since_epoch().count()) *
            new_gain
            ) >> gainShift
        ));

        return (last_pair.slow.time_since_epoch() + leapsecond_offset) - now_fast_after_gain;
    }

    typename slow_clock::duration getLeapAdjustment(int64_t new_gain, typename slow_clock::duration new_bias)
    {
        // Check and update for leap second.
        let prev_fast_as_slow = convert(last_pair.fast);
        let next_fast_as_slow = convert(new_gain, new_bias, last_pair.fast);
        let diff_fast_as_slow = prev_fast_as_slow - next_fast_as_slow;

        return
            (diff_fast_as_slow >= 999ms && diff_fast_as_slow <= 1001ms) ?
            -1s :
            (diff_fast_as_slow >= -1001ms && diff_fast_as_slow <= -999ms) ?
            +1s :
            0s;
    }

    typename slow_clock::duration getCalibrationDifference() noexcept {
        let now_fast_as_slow = convert(last_pair.fast);
        return now_fast_as_slow - last_pair.slow;
    }

    void calibrate() noexcept {
        addCalibrationPoint();
        let diff = getCalibrationDifference();

        let do_gain_calibration = nr_calibrations < 5;

        let new_gain = do_gain_calibration ? getGain() : gain;
        let new_bias = getBias(new_bias);
        let leap_adjustment = getLeapAdjustment(new_gain, new_bias);

        if (do_gain_calibration) {
            LOG_AUDIT("Clock calibration {}: offset={:+} ns gain={:+.15} ns/tick",
                calibration_nr, diff / 1ns, new_gain / gainMultiplier
            );
        } else {
            LOG_AUDIT("Clock calibration {}: offset={:+} ns",
                calibration_nr, diff / 1ns
            );
        }
        
        gain.store(new_gain, std::memory_order_relaxed);
        bias.store(new_bias + leap_adjustment, std::memory_order_relaxed);
        leapsecond_offset += leap_adjustment;
    }

    void calibrate_loop() noexcept {
        while (!calibrate_loop_stop) {
            calibrate();

            auto backoff = calibrate_loop_count++ * 10s;
            if (backoff > 120s) {
                backoff = 120s;
            }

            for (int i = 0; i < (backoff / 100ms); i++) {
                if (calibrate_loop_stop) {
                    break;
                }
                std::this_thread::sleep_for(100ms);
            }
        }
        return;
    }

    typename slow_clock::time_point convert(int64_t new_gain, typename slow_clock::duration new_bias, typename fast_clock::time_point fast_time) const noexcept {
        auto u128_count = static_cast<boost::multiprecision::int128_t>(fast_time.time_since_epoch().count());
        u128_count *= new_gain;
        u128_count >>= gainShift;

        let slow_period = new_bias + typename slow_clock::duration(static_cast<typename slow_clock::rep>(u128_count));
        let slow_time_point = typename slow_clock::time_point(slow_period);
        return slow_time_point;
    }
};

template<typename C1, typename C2>
inline sync_clock_calibration_type<C1,C2> *sync_clock_calibration = nullptr;

/*! A clock which converts one clock to another clock.
 * The new clock is simular to C1 (slow clock), except that leap seconds from C1 are filtered out.
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

    static time_point now() noexcept {
        return convert(fast_clock::now());
    }
};

}
