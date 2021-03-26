// Copyright Take Vos 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include "unfair_mutex.hpp"
#include <date/tz.h>
#include <array>
#include <atomic>
#include <chrono>

namespace tt {
class time_stamp_count;

/** Timestamp
 */
struct hires_utc_clock {
    using rep = int64_t;
    using period = std::nano;
    using duration = std::chrono::duration<rep, period>;
    using time_point = std::chrono::time_point<hires_utc_clock>;
    static const bool is_steady = false;

    /** Get the current time.
     */
	[[nodiscard]] static time_point now() noexcept;

    /** Get the current time and TSC value.
     * @pre Use `set_thread_affinity()` to set the CPU affinity to a single CPU.
     */
    [[nodiscard]] static time_point now(time_stamp_count &tsc) noexcept;

    /** Make a time point from a time stamp count.
     * This function will work in two modes:
     *  - subsystem off: Uses now() and the time_stamp_count frequency to
     *    estimate a timepoint from the given tsc.
     *  - subsystem on: Uses the calibrated TSC offset and more accurate
     *    frequency to estimate a timepoint from the given tsc.
     *    this is much faster and a lot more accurate.
     */
    [[nodiscard]] static time_point make(time_stamp_count const &tsc) noexcept;

    /** This will start the calibration subsystem.
     */
    static void subsystem_start() noexcept;

    /** This will stop the calibration subsystem.
     */
    static void subsystem_stop() noexcept;

    /** A calibration step which will drift the per-cpu tsc-offset.
     * This is a fast lock-free function that may be called from any
     * thread. It is useful to call this from the render thread
     * which means small adjustments to the calibrations are made at
     * 60 fps.
     */
    static void perform_drift() noexcept;

private:
    /** Subsystem initializer.
     */
    static bool subsystem_init() noexcept;

    /** Subsystem de_initializer.
     */
    static void subsystem_deinit() noexcept;

    [[nodiscard]] static size_t find_cpu_id(uint32_t cpu_id) noexcept;

    struct calibration_type {
        long long drift;

        /** The offset in nanosecods to apply to time_stamp_count.
         */
        std::atomic<long long> offset;
    };

    static inline unfair_mutex mutex;
    static inline std::array<uint32_t,64> cpu_ids;
    static inline std::array<calibration_type,64> calibrations;
};

std::string format_engineering(hires_utc_clock::duration duration);

/** Return a ISO-8601 formated date-time.
 * @param utc_timestamp The time_point to format.
 */
std::string format_iso8601_utc(hires_utc_clock::time_point utc_timestamp) noexcept;

/** Return a ISO-8601 formated date-time.
 * @param utc_timestamp The time_point to format.
 * @param time_zone If time_zone is a nullptr then the current timezone is used.
 */
std::string format_iso8601(hires_utc_clock::time_point utc_timestamp, date::time_zone const *time_zone = nullptr) noexcept;

}
