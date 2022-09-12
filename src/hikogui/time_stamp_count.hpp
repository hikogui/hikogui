// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "architecture.hpp"
#include "cast.hpp"
#include "int_carry.hpp"
#include "thread.hpp"
#include <atomic>
#include <array>
#include <cstdint>

#if HI_OPERATING_SYSTEM == HI_OS_WINDOWS
#include <intrin.h>
#elif HI_OPERATING_SYSTEM == HI_OS_LINUX
#include <x86intrin.h>
#endif

namespace hi::inline v1 {

/**
 * Since Window's 10 QueryPerformanceCounter() counts at only 10MHz which
 * is too low to measure performance in many cases.
 *
 * Instead we will use the TSC.
 */
class time_stamp_count {
public:
    struct inplace {
    };
    struct inplace_with_cpu_id {
    };
    struct inplace_with_thread_id {
    };

    constexpr time_stamp_count() noexcept : _count(0), _aux(0), _thread_id(0) {}

    constexpr time_stamp_count(uint64_t count, uint32_t aux) noexcept : _count(count), _aux(aux), _thread_id(0) {}

    /** Use a constructor to in-place create the timestamp.
     */
    explicit time_stamp_count(time_stamp_count::inplace) noexcept : _aux(0), _thread_id(0)
    {
        if constexpr (processor::current == processor::x64) {
            uint32_t tmp;
            _count = __rdtscp(&tmp);
        } else {
            hi_not_implemented();
        }
    }

    /** Use a constructor to in-place create the timestamp.
     */
    explicit time_stamp_count(time_stamp_count::inplace_with_cpu_id) noexcept : _thread_id(0)
    {
        if constexpr (processor::current == processor::x64) {
            _count = __rdtscp(&_aux);
        } else {
            hi_not_implemented();
        }
    }

    /** Use a constructor to in-place create the timestamp.
     */
    explicit time_stamp_count(time_stamp_count::inplace_with_thread_id) noexcept
    {
        if constexpr (processor::current == processor::x64) {
            constexpr uint64_t NT_TIB_CurrentThreadID = 0x48;

            _count = __rdtscp(&_aux);
            _thread_id = __readgsdword(NT_TIB_CurrentThreadID);
        } else {
            hi_not_implemented();
        }
    }

    /** Get the current count from the CPU's time stamp count.
     * @return The current clock count and CPU-id.
     */
    [[nodiscard]] static time_stamp_count now() noexcept
    {
        return time_stamp_count{time_stamp_count::inplace_with_cpu_id{}};
    }

    /** Get the logical CPU index.
     * This is logical CPU id that the operating system uses for things
     * like thread affinity.
     *
     * @return the processor index, or -1 if the processor index is unknown.
     */
    [[nodiscard]] ssize_t cpu_id() const noexcept
    {
        if (_aux_is_cpu_id.load(std::memory_order::relaxed)) {
            // On Linux the upper bits are used for a node-id.
            return _aux & 0xfff;
        } else {
            return cpu_id_fallback();
        }
    }

    /** Get the thread id.
     * @pre Must call `time_stamp_count(time_stamp_count::inplace_with_thread_id)` first.
     */
    [[nodiscard]] constexpr uint32_t thread_id() const noexcept
    {
        return _thread_id;
    }

    /** Get the count since epoch.
     * The epoch is the same as the TSC count's epoch. In most cases the epoch
     * is at system startup time.
     */
    [[nodiscard]] constexpr uint64_t count() const noexcept
    {
        return _count;
    }

    /** Convert a time-stamp count to a duration.
     *
     * @param count The number clock ticks.
     * @return A period of nanoseconds representing the count of clock ticks.
     */
    [[nodiscard]] static std::chrono::nanoseconds duration_from_count(uint64_t count) noexcept
    {
        using namespace std::chrono_literals;

        hilet[lo, hi] = mul_carry(count, _period.load(std::memory_order::relaxed));
        return 1ns * static_cast<int64_t>((hi << 32) | (lo >> 32));
    }

    /** Convert to nanoseconds since epoch.
     * The epoch is the same as the TSC count's epoch. In most cases the epoch
     * is at system startup time.
     */
    [[nodiscard]] std::chrono::nanoseconds time_since_epoch() const noexcept
    {
        return duration_from_count(_count);
    }

    constexpr time_stamp_count &operator+=(uint64_t rhs) noexcept
    {
        _count += rhs;
        return *this;
    }

    [[nodiscard]] constexpr time_stamp_count operator+(uint64_t rhs) const noexcept
    {
        auto tmp = *this;
        tmp += rhs;
        return tmp;
    }

    /** Measure the frequency of the time_stamp_count.
     * Frequency drift from TSC is 1ppm
     */
    [[nodiscard]] static uint64_t measure_frequency(std::chrono::milliseconds duration) noexcept;

    static void set_frequency(uint64_t frequency) noexcept
    {
        hilet period = (uint64_t{1'000'000'000} << 32) / frequency;
        _period.store(period, std::memory_order_relaxed);
    }

    /** Start the time_stamp_count subsystem.
     */
    static void start_subsystem() noexcept;

private:
    uint64_t _count;

    /** On Intel x64 this is the TSC_AUX register value for this
     * CPU. The operating system writes this value and is often not document.
     *
     * We check if the lower 12 bits match the logical CPU id to use the fast
     * pad for aux value to CPU id conversion. Otherwise we keep track in a table
     * of each aux value and CPU id.
     */
    uint32_t _aux;

    /** A struct packing optimization, add the thread id in this same struct.
     */
    uint32_t _thread_id;

    /** The period in nanoseconds/cycle as Q32.32
     */
    inline static std::atomic<uint64_t> _period = 0;

    inline static std::atomic<bool> _aux_is_cpu_id = false;

    /** The number of CPU ids we know of.
     */
    inline static std::atomic<std::size_t> _num_aux_values = 0;

    /** A list of known CPU ids.
     */
    inline static std::array<uint32_t, maximum_num_cpus> _aux_values;

    /** A list of CPU ids that match the _aux_values list.
     */
    inline static std::array<std::size_t, maximum_num_cpus> _cpu_ids;

    /** Get the CPU id.
     * This is logical CPU id that the operating system uses.
     * This is the fallback function that will search through the
     * table of _aux_values.
     *
     * @return the CPU id, or -1 if the CPU id is unknown.
     */
    [[nodiscard]] ssize_t cpu_id_fallback() const noexcept;

    static void populate_aux_values() noexcept;
    static void configure_frequency() noexcept;
};

} // namespace hi::inline v1
