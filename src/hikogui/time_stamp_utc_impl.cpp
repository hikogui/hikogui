// Copyright Take Vos 2019, 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "time_stamp_utc.hpp"
#include "time_stamp_count.hpp"
#include "log.hpp"
#include "thread.hpp"
#include <format>
#include <bit>
#include <iterator>
#include <algorithm>
#include <numeric>
#include <chrono>

namespace hi::inline v1 {

std::string format_engineering(std::chrono::nanoseconds duration)
{
    using namespace std::chrono_literals;

    if (duration >= 1s) {
        return std::format("{:.3g}s ", static_cast<double>(duration / 1ns) / 1'000'000'000);
    } else if (duration >= 1ms) {
        return std::format("{:.3g}ms", static_cast<double>(duration / 1ns) / 1'000'000);
    } else if (duration >= 1us) {
        return std::format("{:.3g}us", static_cast<double>(duration / 1ns) / 1'000);
    } else {
        return std::format("{:.3g}ns", static_cast<double>(duration / 1ns));
    }
}

[[nodiscard]] utc_nanoseconds time_stamp_utc::now(time_stamp_count &tsc) noexcept
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
            hi_log_fatal("CPU Switch detected during get_sample(), which should never happen");
        }

        if (tmp_tsc1.count() > tmp_tsc2.count()) {
            hi_log_warning("TSC skipped backwards");
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
        hi_log_fatal("Unable to get TSC sample.");
    }

    tsc = shortest_tsc;
    return shortest_tp;
}

[[nodiscard]] utc_nanoseconds time_stamp_utc::make(time_stamp_count const &tsc) noexcept
{
    auto i = tsc.cpu_id();
    if (i >= 0) {
        hilet tsc_epoch = tsc_epochs[i].load(std::memory_order::relaxed);
        if (tsc_epoch != utc_nanoseconds{}) {
            return tsc_epoch + tsc.time_since_epoch();
        }
    }

    // Fallback.
    hilet ref_tp = std::chrono::utc_clock::now();
    hilet ref_tsc = time_stamp_count::now();
    hilet diff_ns = ref_tsc.time_since_epoch() - tsc.time_since_epoch();
    return ref_tp - diff_ns;
}

void time_stamp_utc::subsystem_proc_frequency_calibration(std::stop_token stop_token) noexcept
{
    using namespace std::chrono_literals;

    // Calibrate the TSC frequency to within 1 ppm.
    // A 1s measurement already brings is to about 1ppm. We are
    // going to be taking average of the IQR of 11 samples, just
    // in case there are UTC clock adjustment made during the measurement.

    std::array<uint64_t, 16> frequencies;
    for (auto i = 0; i != frequencies.size();) {
        hilet f = time_stamp_count::measure_frequency(1s);
        if (f != 0) {
            frequencies[i] = f;
            ++i;
        }

        if (stop_token.stop_requested()) {
            return;
        }
    }
    std::ranges::sort(frequencies);
    hilet iqr_size = frequencies.size() / 2;
    hilet iqr_first = std::next(frequencies.cbegin(), frequencies.size() / 4);
    hilet iqr_last = std::next(iqr_first, iqr_size);
    hilet frequency = std::accumulate(iqr_first, iqr_last, uint64_t{0}) / iqr_size;

    hi_log_info("Accurate measurement of TSC frequency result is {} Hz", frequency);
    time_stamp_count::set_frequency(frequency);
}

static void advance_cpu_thread_mask(uint64_t const &process_cpu_mask, uint64_t &thread_cpu_mask)
{
    hi_assert(std::popcount(process_cpu_mask) > 0);
    hi_assert(std::popcount(thread_cpu_mask) == 1);

    do {
        if ((thread_cpu_mask <<= 1) == 0) {
            thread_cpu_mask = 1;
        }

    } while ((process_cpu_mask & thread_cpu_mask) == 0);
}

void time_stamp_utc::subsystem_proc(std::stop_token stop_token) noexcept
{
    using namespace std::chrono_literals;

    set_thread_name("time_stamp_utc");
    subsystem_proc_frequency_calibration(stop_token);

    hilet process_cpu_mask = process_affinity_mask();

    std::size_t next_cpu = 0;
    while (not stop_token.stop_requested()) {
        hilet current_cpu = advance_thread_affinity(next_cpu);

        std::this_thread::sleep_for(100ms);
        hilet lock = std::scoped_lock(time_stamp_utc::mutex);

        time_stamp_count tsc;
        hilet tp = time_stamp_utc::now(tsc);
        hi_assert(tsc.cpu_id() == narrow_cast<ssize_t>(current_cpu));

        tsc_epochs[current_cpu].store(tp - tsc.time_since_epoch(), std::memory_order::relaxed);
    }
}

[[nodiscard]] bool time_stamp_utc::init_subsystem() noexcept
{
    time_stamp_utc::subsystem_thread = std::jthread{subsystem_proc};
    return true;
}

void time_stamp_utc::deinit_subsystem() noexcept
{
    if (global_state_disable(global_state_type::time_stamp_utc_is_running)) {
        if (time_stamp_utc::subsystem_thread.joinable()) {
            time_stamp_utc::subsystem_thread.request_stop();
            time_stamp_utc::subsystem_thread.join();
        }
    }
}

bool time_stamp_utc::start_subsystem() noexcept
{
    return hi::start_subsystem(global_state_type::time_stamp_utc_is_running, init_subsystem, deinit_subsystem);
}

void time_stamp_utc::stop_subsystem() noexcept
{
    return hi::stop_subsystem(deinit_subsystem);
}

} // namespace hi::inline v1
