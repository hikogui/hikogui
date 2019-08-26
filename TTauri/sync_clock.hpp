// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "hires_utc_clock.hpp"
#include "cpu_counter_clock.hpp"
#include <boost/multiprecision/cpp_int.hpp>
#include <chrono>

using namespace std::literals::chrono_literals;

namespace TTauri {

template<typename C1, typename C2>
class sync_clock_impl {
    using slow_clock = C1;
    using fast_clock = C2;

    /*! For lock-free access, this should be 128 bit in size
     */
    struct calibration_t {
        static constexpr int gainShift = 60;
        static constexpr double gainMultiplier = static_cast<double>(1ULL << gainShift);

        int64_t gain = 0;
        typename slow_clock::duration bias = 0ns;
    };

    alignas(16) std::atomic<calibration_t> calibration;

    /*! Previous time point so we can calculate the gain value over the period since the last
     * calibration.
     */
    std::pair<typename slow_clock::time_point, typename fast_clock::time_point> previousTimepoints;

    /*! A list of previous gain calibration values. This list is averaged (through IQR arithmatic mean)
     * and then used as the calibration value. Because of using the IQR we can ignore anomalies due to
     * leap seconds.
     */
    std::array<double, 20> gains;
    size_t gainCount = 0;

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
    sync_clock_impl(bool create_thread=true) noexcept {
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

    typename slow_clock::time_point convert(typename fast_clock::time_point fast_time) const noexcept {
        return convert(calibration.load(), fast_time);
    }

private:
    void calibrate_loop() noexcept {
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

    calibration_t calibrate(typename slow_clock::time_point now_slow, typename fast_clock::time_point now_fast) noexcept {
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

        let now_fast_after_gain = typename slow_clock::duration(static_cast<int64_t>(
            (
                static_cast<boost::multiprecision::int128_t>(now_fast.time_since_epoch().count()) *
                new_gain
            ) >> calibration_t::gainShift
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

    typename slow_clock::time_point convert(calibration_t const c, typename fast_clock::time_point fast_time) const noexcept {
        auto u128_count = static_cast<boost::multiprecision::int128_t>(fast_time.time_since_epoch().count());
        u128_count *= c.gain;
        u128_count >>= calibration_t::gainShift;

        let slow_period = c.bias + typename slow_clock::duration(static_cast<typename slow_clock::rep>(u128_count));
        let slow_time_point = typename slow_clock::time_point(slow_period);
        return slow_time_point;
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
    using impl = sync_clock_impl<C1,C2>;

    using rep = typename slow_clock::rep;
    using period = typename slow_clock::period;
    using duration = typename slow_clock::duration;
    using time_point = typename slow_clock::time_point;
    static const bool is_steady = slow_clock::is_steady;

    static duration checkCalibration() noexcept {
        let now_slow = slow_clock::now();
        let now_fast = fast_clock::now();
        let now_fast_as_slow = convert(now_fast);
        return now_fast_as_slow - now_slow;
    }

    /*! Return a timestamp from a clock.
     */
    static time_point convert(typename fast_clock::time_point fast_time) noexcept {
        return get_singleton<impl>().convert(fast_time);
    }

    static time_point now() noexcept {
        return convert(fast_clock::now());
    }
};

using hiperf_utc_clock = sync_clock<hires_utc_clock,cpu_counter_clock>;

}
