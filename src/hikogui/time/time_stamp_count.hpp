// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "chrono.hpp"
#include "../utility/utility.hpp"
#include "../concurrency/concurrency.hpp"
#include "../numeric/numeric.hpp"
#include "../macros.hpp"
#include <atomic>
#include <array>
#include <cstdint>
#include <chrono>
#include <utility>
#include <thread>

#if HI_OPERATING_SYSTEM == HI_OS_WINDOWS
#include <intrin.h>
#elif HI_OPERATING_SYSTEM == HI_OS_LINUX
#include <x86intrin.h>
#endif

hi_export_module(hikogui.time.time_stamp_count);


hi_export namespace hi::inline v1 {

/**
 * Since Window's 10 QueryPerformanceCounter() counts at only 10MHz which
 * is too low to measure performance in many cases.
 *
 * Instead we will use the TSC.
 */
class time_stamp_count {
public:
    struct inplace {};
    struct inplace_with_cpu_id {};
    struct inplace_with_thread_id {};

    constexpr time_stamp_count() noexcept : _count(0), _aux(0), _thread_id(0) {}

    constexpr time_stamp_count(uint64_t count, uint32_t aux) noexcept : _count(count), _aux(aux), _thread_id(0) {}

    /** Use a constructor to in-place create the timestamp.
     */
    explicit time_stamp_count(time_stamp_count::inplace) noexcept : _aux(0), _thread_id(0)
    {
#if HI_PROCESSOR == HI_CPU_X86_64
        uint32_t tmp;
        _count = __rdtscp(&tmp);
#else
#error "Not Implemented"
#endif
    }

    /** Use a constructor to in-place create the timestamp.
     */
    explicit time_stamp_count(time_stamp_count::inplace_with_cpu_id) noexcept : _thread_id(0)
    {
#if HI_PROCESSOR == HI_CPU_X86_64
            _count = __rdtscp(&_aux);
#else
#error "Not Implemented"
#endif
    }

    /** Use a constructor to in-place create the timestamp.
     */
    explicit time_stamp_count(time_stamp_count::inplace_with_thread_id) noexcept
    {
#if HI_PROCESSOR == HI_CPU_X86_64 and HI_OPERATING_SYSTEM == HI_OS_WINDOWS
        constexpr uint64_t NT_TIB_CurrentThreadID = 0x48;

        _count = __rdtscp(&_aux);
        _thread_id = __readgsdword(NT_TIB_CurrentThreadID);
#else
#error "Not Implemented"
#endif
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

    constexpr time_stamp_count& operator+=(uint64_t rhs) noexcept
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

    /** Get a good quality time sample.
     *
     * @pre The CPU affinity must be set to a single CPU.
     * @return The current UTC time in nanoseconds, the current time-stamp count.
     * @throw os_error When there is a problem getting a time-sample.
     */
    [[nodiscard]] static std::pair<utc_nanoseconds, time_stamp_count> time_stamp_utc_sample()
    {
        auto shortest_diff = std::numeric_limits<uint64_t>::max();
        time_stamp_count shortest_tsc;
        utc_nanoseconds shortest_tp;

        // With three samples gathered on the same CPU we should
        // have a TSC/UTC/TSC combination that was run inside a single time-slice.
        for (auto i = 0; i != 10; ++i) {
            hilet tmp_tsc1 = time_stamp_count::now();
            hilet tmp_tp = std::chrono::utc_clock::now();
            hilet tmp_tsc2 = time_stamp_count::now();

            if (tmp_tsc1.cpu_id() != tmp_tsc2.cpu_id()) {
                throw os_error("CPU Switch detected during get_sample(), which should never happen");
            }

            if (tmp_tsc1.count() > tmp_tsc2.count()) {
                // TSC skipped backwards, this may happen when the TSC of multiple
                // CPUs get synchronized with each other.
                // For example when waking up from sleep.
                continue;
            }

            hilet diff = tmp_tsc2.count() - tmp_tsc1.count();

            if (diff < shortest_diff) {
                shortest_diff = diff;
                shortest_tp = tmp_tp;
                shortest_tsc = tmp_tsc1 + (diff / 2);
            }
        }

        if (shortest_diff == std::numeric_limits<uint64_t>::max()) {
            throw os_error("Unable to get TSC sample.");
        }

        return {shortest_tp, shortest_tsc};
    }

    /** Measure the frequency of the time_stamp_count.
     * Frequency drift from TSC is 1ppm
     *
     * @param sample_duration The time between samples to determine the frequency
     *                        longer duration gives a better quality but may
     *                        increase application start-up time.
     * @return The clock frequency of the TSC in Hz.
     */
    [[nodiscard]] static uint64_t measure_frequency(std::chrono::milliseconds sample_duration)
    {
        using namespace std::chrono_literals;

        // Only sample the frequency of one of the TSC clocks.
        hilet prev_mask = set_thread_affinity(current_cpu_id());

        hilet [tp1, tsc1] = time_stamp_utc_sample();
        std::this_thread::sleep_for(sample_duration);
        hilet [tp2, tsc2] = time_stamp_utc_sample();

        // Reset the mask back.
        set_thread_affinity_mask(prev_mask);

        if (tsc1._aux != tsc2._aux) {
            // This must never happen, as we set the thread affinity to a single CPU
            // if this happens something is seriously wrong.
            throw os_error("CPU Switch detected when measuring the TSC frequency.");
        }

        if (tsc1.count() >= tsc2.count()) {
            // The TSC should only be reset during the very early boot sequence when
            // the CPUs are started and synchronized. It may also happen to a CPU that
            // was hot-swapped while the computer is running, in that case the CPU
            // should not be running applications yet.
            throw os_error("TSC Did not advance during measuring its frequency.");
        }

        if (tp1 >= tp2) {
            // The UTC clock did not advance, maybe a time server changed the clock.
            return 0;
        }

        // Calculate the frequency by dividing the delta-tsc by the duration.
        // We scale both the delta-tsc and duration by 1'000'000'000 before the
        // division. The duration is scaled by 1'000'000'000 by dividing by 1ns.
        hilet[delta_tsc_lo, delta_tsc_hi] = mul_carry(tsc2.count() - tsc1.count(), uint64_t{1'000'000'000});
        auto duration = narrow_cast<uint64_t>((tp2 - tp1) / 1ns);
        return wide_div(delta_tsc_lo, delta_tsc_hi, duration);
    }

    static void set_frequency(uint64_t frequency) noexcept
    {
        hilet period = (uint64_t{1'000'000'000} << 32) / frequency;
        _period.store(period, std::memory_order_relaxed);
    }

    /** Start the time_stamp_count subsystem.
     *
     * @return The time-stamp-counter frequency
     * @throw os_error When the time-stamp-counter does not work. AUX is the same
     *                 as the cpu-id.
     */
    static std::pair<uint64_t, bool> start_subsystem()
    {
        hilet frequency = configure_frequency();
        hilet aux_is_cpu_id = populate_aux_values();
        return {frequency, aux_is_cpu_id};
    }

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
    [[nodiscard]] ssize_t cpu_id_fallback() const noexcept
    {
        auto aux_value_ = _mm_set1_epi32(_aux);

        hilet num_aux_values = _num_aux_values.load(std::memory_order_acquire);
        hi_assert(_aux_values.size() == _cpu_ids.size());
        hi_assert_bounds(num_aux_values, _aux_values);

        for (std::size_t i = 0; i < num_aux_values; i += 4) {
            hilet row = _mm_loadu_si128(reinterpret_cast<__m128i const *>(_aux_values.data() + i));
            hilet row_result = _mm_cmpeq_epi32(row, aux_value_);
            hilet row_result_ = _mm_castsi128_ps(row_result);
            hilet row_result_mask = _mm_movemask_ps(row_result_);
            if (to_bool(row_result_mask)) {
                hilet j = i + std::countr_zero(narrow_cast<unsigned int>(row_result_mask));
                if (j < num_aux_values) {
                    return _cpu_ids[j];
                }

                return -1;
            }
        }

        return -1;
    }

    static bool populate_aux_values()
    {
        // Keep track of the original thread affinity of the main thread.
        auto prev_mask = set_thread_affinity(current_cpu_id());

        // Create a table of cpu_ids.
        std::size_t next_cpu = 0;
        std::size_t current_cpu = 0;
        bool aux_is_cpu_id = true;
        do {
            current_cpu = advance_thread_affinity(next_cpu);

            auto i = _num_aux_values.load(std::memory_order::acquire);
            auto tsc = time_stamp_count::now();
            _aux_values[i] = tsc._aux;
            _cpu_ids[i] = current_cpu;
            _num_aux_values.store(i + 1, std::memory_order::release);

            if ((tsc._aux & 0xfff) != current_cpu) {
                aux_is_cpu_id = false;
            }

        } while (next_cpu > current_cpu);

        _aux_is_cpu_id.store(aux_is_cpu_id, std::memory_order_relaxed);

        // Set the thread affinity back to the original.
        set_thread_affinity_mask(prev_mask);
        return aux_is_cpu_id;
    }
    static uint64_t configure_frequency()
    {
        using namespace std::chrono_literals;

        // This function is called from the crt and must therefor be quick as we do not
        // want to keep the user waiting. We are satisfied if the measured frequency is
        // to within 1% accuracy.

        // We take an average over 4 times in case the hires_utc_clock gets reset by a time server.
        uint64_t frequency = 0;
        uint64_t num_samples = 0;
        for (int i = 0; i != 4; ++i) {
            hilet f = time_stamp_count::measure_frequency(25ms);
            if (f != 0) {
                frequency += f;
                ++num_samples;
            }
        }
        if (num_samples == 0) {
            throw os_error("Unable the measure the frequency of the TSC. The UTC time did not advance.");
        }
        frequency /= num_samples;

        time_stamp_count::set_frequency(frequency);
        return frequency;
    }
};

} // namespace hi::inline v1
