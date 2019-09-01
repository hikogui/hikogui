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

    static constexpr int gainShift = 60;
    static constexpr double gainMultiplier = static_cast<double>(1ULL << gainShift);
    static constexpr size_t shared_time_points_size = 32;
    static constexpr int minimum_time_points_until_stable_gain = 8;

    std::atomic<int64_t> gain = 0;
    std::atomic<typename slow_clock::duration> bias = 0ns;

    /*! When during calibration we detect a leap second, we will update this offset (in ns).
    */
    typename slow_clock::duration leapsecond_offset = 0ns;

    struct shared_time_point_type {
        typename slow_clock::time_point slow_time_point;
        typename fast_clock::time_point fast_time_point;
        double gain;
    };

    static double calculate_gain(shared_time_point_type const &first, shared_time_point_type const &second) {
        // Calculate the gain between the current and previous calibration.
        // Then store this gain into a fifo.
        let diff_slow = second.slow_time_point - first.slow_time_point;
        let diff_fast = second.fast_time_point - first.fast_time_point;
        return static_cast<double>(diff_slow.count()) / static_cast<double>(diff_fast.count());
    }


    /*! A ring buffer of previous time points used for calculating gain and bias.
     */
    std::array<shared_time_point_type, shared_time_points_size> shared_time_points;
    size_t shared_time_points_count = 0;


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
    void calibrate_loop() noexcept {
        while (!calibrate_loop_stop) {
            let iteration = increment_counter<"calibrate_clk"_tag>();
            LOG_AUDIT("Clock calibration: iteration={}, offset={:+} ns", iteration, checkCalibration().count());
            calibrate(slow_clock::now(), fast_clock::now());

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

    typename slow_clock::duration checkCalibration() noexcept {
        let now_slow = slow_clock::now();
        let now_fast = fast_clock::now();
        let now_fast_as_slow = convert(now_fast);
        return now_fast_as_slow - now_slow;
    }

    void add_shared_time_point(typename slow_clock::time_point now_slow, typename fast_clock::time_point now_fast) noexcept {
        auto next_shared_time_point = shared_time_point_type{now_slow, now_fast, 0.0};

        if (shared_time_points_count > 0) {
            auto &previous_shared_time_point = shared_time_points[shared_time_points_count - 1 % shared_time_points_size];
            let tmp_gain = calculate_gain(previous_shared_time_point, next_shared_time_point);

            // Simplify calculations, by updating both the previous and next shared time point with the calculated gain.
            previous_shared_time_point.gain = tmp_gain;
            next_shared_time_point.gain = tmp_gain;
        }

        shared_time_points[shared_time_points_count++ % shared_time_points_size] = next_shared_time_point;
    }

    int64_t calibrate_gain() noexcept {
        if (shared_time_points_count < 2) {
            // No gain to calculate yet.
            return static_cast<int64_t>(1.0 * gainMultiplier + 0.5);
        }
        
        std::array<double, shared_time_points_size> gains;

        // Copy the gain values from the shared_time_points for sorting.
        std::transform(shared_time_points.begin(), shared_time_points.end(), gains.begin(), [](auto x) {
            return x.gain;
        });

        // Calculate the interquartiel range.
        let gain_count = std::min(shared_time_points_count, shared_time_points_size);
        std::sort(gains.begin(), gains.begin() + gain_count);
        let iqr_begin = gains.begin() + (gain_count / 4);
        let iqr_end = gains.begin() + ((gain_count * 3) / 4);
        let iqr_length = iqr_end - iqr_begin;

        // Calculate the arithmatic mean over the inter-quartiel-range,
        // Or if the inter-quartiel-range is too small: the artihmatic mean over the whole gain table.
        let mean_gain = (iqr_length > 3) ?
            std::accumulate(iqr_begin, iqr_end, 0.0) / static_cast<double>(iqr_length) :
            std::accumulate(gains.begin(), gains.begin() + gain_count, 0.0) / static_cast<double>(gain_count);

        LOG_INFO("Calibrating clock: gain={:+.15} nanosecond/cpu-tick", mean_gain); 
        return static_cast<int64_t>(mean_gain * gainMultiplier + 0.5);
    }

    typename slow_clock::duration calibrate_bias(int64_t new_gain, typename slow_clock::time_point now_slow, typename fast_clock::time_point now_fast) noexcept {
        // Calculate the new calibration values.
        let now_fast_after_gain = typename slow_clock::duration(static_cast<int64_t>(
            (
                static_cast<boost::multiprecision::int128_t>(now_fast.time_since_epoch().count()) *
                new_gain
                ) >> gainShift
            ));

        return (now_slow.time_since_epoch() + leapsecond_offset) - now_fast_after_gain;
    }

    typename slow_clock::duration calibrate_leapsecond_adjustment(int64_t new_gain, typename slow_clock::duration new_bias, typename fast_clock::time_point now_fast)
    {
        // Check and update for leap second.
        let prev_fast_as_slow = convert(now_fast);
        let next_fast_as_slow = convert(new_gain, new_bias, now_fast);
        let diff_fast_as_slow = prev_fast_as_slow - next_fast_as_slow;

        return
            (diff_fast_as_slow >= 999ms && diff_fast_as_slow <= 1001ms) ?
            -1s :
            (diff_fast_as_slow >= -1001ms && diff_fast_as_slow <= -999ms) ?
            +1s :
            0s;
    }

    void calibrate(typename slow_clock::time_point now_slow, typename fast_clock::time_point now_fast) noexcept {
        add_shared_time_point(now_slow, now_fast);

        let new_gain = (shared_time_points_count <= minimum_time_points_until_stable_gain) ? calibrate_gain() : gain.load(std::memory_order_relaxed);
        let new_bias = calibrate_bias(new_gain, now_slow, now_fast);
        let leapsecond_adjustment = calibrate_leapsecond_adjustment(new_gain, new_bias, now_fast);

        gain.store(new_gain, std::memory_order_relaxed);
        bias.store(new_bias + leapsecond_adjustment, std::memory_order_relaxed);
        leapsecond_offset += leapsecond_adjustment;
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
