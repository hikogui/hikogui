
#pragma once

#include "bigint.hpp"
#include <atomic>
#include <emmintrin.h>

namespace tt {

template<typename Clock>
class tsc_to_timepoint {
public:
    using clock_type = Clock;
    using time_point = typename clock_type::time_point;
    using duration = typename clock_type::duration;

    constexpr tsc_to_timepoint() noexcept :
        version1(0), offset(0), period(0), version2(0) {}

    inline operator time_point ()(int64_t counter) noexcept
    {
        uint64_t offset;
        ubig128 period;
        offset_and_period(offset, period);

        auto tmp = ubig128{counter} * period;
        tmp += (1ULL << 63);
        tmp >>= 64;

        return time_point{tmp} - offset;
    }
  
    /** Add a calibration point.
     * @param tp The time point.
     * @param count The counter value.
     */ 
    inline void calibrate(time_point tp, uint64_t count) noexcept
    {
        ttlet lock = std::scoped_lock(mutex);

    }

private:
    /** This mutex should be held when adding calibration data.
     */
    mutable std::mutex mutex;

    /** The version number of the calibration values.
     * When the version number is odd, a thread is writing the calibration
     * values, when it is even the calibration values are not being written.
     *
     * When reading if the version is even and remains the same, during the
     * reading of the calibration, the read succeeds.
     */
    std::atomic<uint32_t> _version;

    duration _offset;

    /** Period in fixed-64.64 format.
     * The period describes the number of nanoseconds per clock-tick.
     */
    ubig128_t _period;

    inline void set_offset_and_period(uint64_t offset, ubig128 period) const noexcept
    {
        _version.fetch_add(1, std::memory_order::acquire);
        _offset = offset;
        _period = period; 
        _version.fetch_add(1, std::memory_order::release);
    }

    inline void offset_and_period(uint64_t &offset, ubig128 &period) const noexcept
    {
        while (true) {
            uint64_t version1 = _version.load(std::memory_order::acquire);
            offset = _offset;
            period = _period;
            atomic_thread_fence(std::memory_order::release);
            uint64_t version2 = _version.load(std::memory_order::relaxed);

            if (version1 == version2 && (version2 & 1) == 0) {
                return;
            }

            // Tell the processor that we are spinning.
            _mm_pause();
        }
    }

};

}
