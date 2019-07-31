// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <boost/multiprecision/cpp_int.hpp>
#include <chrono>

namespace TTauri::Time {


template<typename C1, typename C2>
class sync_clock_impl {
    using slow_clock = C1;
    using fast_clock = C2;

    /*! For lock-free access, this should be 128 bit in size
     */
    alignas(16)
    struct calibration_t {
        static constexpr int gainShift = 60;
        static constexpr double gainMultiplier = static_cast<double>(1ULL << gainShift);

        uint64_t gain = 0;
        slow_clock::duration bias = 0;
    };

    std::atomic<calibration_t> calibration;

    /*! Previous time point so we can calculate the gain value over the period since the last
     * calibration.
     */
    std::pair<typename slow_clock::timepoint, typename fast_clock::timepoint> previousTimepoints;

    /*! A list of previous gain calibration values. This list is averaged (through IQR arithmatic mean)
     * and then used as the calibration value. Because of using the IQR we can ignore anomalies due to
     * leap seconds.
     */
    std::array<double, 20> gains;
    size_t gainCount = 0;

    /*! When during calibration we detect a leap second, we will update this offset (in ns).
     */
    slow_clock::duration leapsecond_offsest = 0;

    std::thread calibrate_loop_id;
    bool calibrate_loop_stop = false;
    size_t calibrate_loop_count = 0;

    /*! Construct a sync clock.
     * \param create_thread can be set to false when testing.
     */
    sync_clock_impl(bool create_thread=true) {
        if (create_thread) {
            calibrate_loop_id = std::thread([&]() {
                return this->calibrate_loop();
            });
        }
    }

    ~sync_clock_impl() {
        calibrate_loop_stop = true;
        if (calibrate_loop_id.joinable()) {
            calibrate_loop_id.join();
        }
    }

    void calibrate_loop() {
        while (!calibrate_loop_stop) {
            if (
                (calibrate_loop_count < 10) ||
                (calibrate_loop_count < 120 && (calibrate_loop_count % 10) == 0) ||
                (calibrate_loop_count % 60 == 0)
            ) {
                calibration.store(calibrate(slow_clock::now(), fast_clock::now()));
            }

            calibrate_loop_count++;
            std::this_thread::sleep_for(1s);
        }
        return;
    }

    calibration_t calibrate(typename slow_clock::timepoint now_slow, typename fast_clock::timepoint now_fast) {
        let [prev_slow, prev_fast] = previousTimepoints;
        previousTimepoints = {now_slow, now_fast};

        // Bail out if we don't have two calibration points.
        if (prev_slow.time_since_epoch() == 0s) {
            return { 0, {} };
        }

        // Calculate the gain between the current and previous calibration.
        // Then store this gain into a fifo.
        let diff_slow = now_slow - prev_slow;
        let diff_fast = now_fast - prev_fast;
        let gain = static_cast<double>(diff_slow.count()) / static_cast<double>(diff_fast.count());
        gains[gainCount++ % gains.max_size()] = gain;

        // Prepare the inter-quartiel-range over all gains in the fifo.
        let gainCountClamped = std::min(gainCount, gains.max_size());
        auto sortedGains = gains;
        std::sort(sortedGains.begin(), sortedGains.begin() + gainCountClamped);
        let iqr_begin = sortedGains.begin() + (gainCountClamped / 4);
        let iqr_end = sortedGains.begin() + ((gainCountClamped * 3) / 4);
        let iqr_length = iqr_end - iqr_begin;

        // Calculate the arithmatic mean over the inter-quartiel-range
        let mean_gain = (iqr_length > 0) ?
            std::accumulate(iqr_begin, iqr_end, 0.0) / static_cast<double>(iqr_length) :
            gain;

        // Calculate the new calibration values.
        let new_gain = static_cast<int64_t>(mean_gain * calibration_t::gainMultiplier + 0.5);

        let now_fast_after_gain = slow_clock::duration(static_cast<int64_t>(
            (
                static_cast<boost::multiprecision::int128_t>(now_fast.time_since_epoch().count()) *
                new_gain
            ) >> sync_clock_calibration_t::gainShift
        ));

        let new_bias = (now_slow.time_since_epoch() + leapsecond_offset) - now_fast_after_gain;

        // Check and update for leap second.
        let now_fast_as_slow = convert(now_fast);
        let new_fast_as_slow = convert({ new_gain, new_bias }, now_fast);
        let diff_fast_as_slow = now_fast_as_slow - new_fast_as_slow;

        let adjustment =
            (diff_fast_as_slow >= 999ms && diff_fast_as_slow <= 1001ms) ?
                -1s :
                (diff_fast_as_slow >= -1001ms && diff_fast_as_slow <= -999ms) ?
                    +1s :
                    0s;

        leapsecond_offset += adjustment;
        return { new_gain, new_bias + adjustment };
    }

    typename slow_clock::timepoint convert(sync_clock_calibration_t const c, typename fast_clock::timepoint fast_time) const {
        auto u128_count = static_cast<boost::multiprecision::int128_t>(fast_time.time_since_epoch().count());
        u128_count *= c.gain;
        u128_count >>= calibration_t::gainShift;
        u128_count += c.bias;

        let i64_count = static_cast<typename rep>(u128_count);
        let slow_period = duration(i64_count);
        let slow_timepoint = timepoint(slow_period);
        return slow_timepoint;
    }

    typename slow_clock::timepoint convert(typename fast_clock::timepoint fast_time) const {
        return convert(calibration.load(), fast_time);
    }
};


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
    using impl = sync_clock_impl<C1,C2>

    using rep = typename slow_clock::rep;
    using period = typename slow_clock::period;
    using duration = typename slow_clock::duration;
    using timepoint = timepoint<sync_clock>;
    static const bool is_steady = slow_clock::is_steady;

    static duration checkCalibration() {
        let now_slow = slow_clock::now();
        let now_fast = fast_clock::now();
        let now_fast_as_slow = convert(now_fast);
        return now_fast_as_slow - now_slow;
    }

    /*! Return a timestamp from a clock.
     */
    static timepoint convert(typename fast_clock::timepoint fast_time) {
        return get_singleton<impl>().convert(fast_time);
    }

    static timepoint now() {
        return clock_sync::convert(fast_clock::now());
    }
};

}
