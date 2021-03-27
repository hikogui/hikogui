// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hires_utc_clock.hpp"
#include "time_stamp_count.hpp"
#include "application.hpp"
#include "logger.hpp"
#include "thread.hpp"
#include <immintrin.h>
#include <fmt/ostream.h>
#include <fmt/format.h>
#include <bit>
#include <iterator>

namespace tt {

using namespace std::chrono_literals;

std::string format_engineering(hires_utc_clock::duration duration)
{
    if (duration >= 1s) {
        return fmt::format("{:.3g} s ", static_cast<double>(duration / 1ns) / 1'000'000'000);
    } else if (duration >= 1ms) {
        return fmt::format("{:.3g} ms", static_cast<double>(duration / 1ns) / 1'000'000);
    } else if (duration >= 1us) {
        return fmt::format("{:.3g} us", static_cast<double>(duration / 1ns) / 1'000);
    } else {
        return fmt::format("{:.3g} ns", static_cast<double>(duration / 1ns));
    }
}

[[nodiscard]] hires_utc_clock::time_point hires_utc_clock::now(time_stamp_count &tsc) noexcept
{
    auto shortest_diff = std::numeric_limits<uint64_t>::max();
    time_stamp_count shortest_tsc;
    hires_utc_clock::time_point shortest_tp;

    // With three samples gathered on the same CPU we should
    // have a TSC/UTC/TSC combination that was run inside a single time-slice.
    for (auto i = 0; i != 10; ++i) {
        ttlet tmp_tsc1 = time_stamp_count::now();
        ttlet tmp_tp = hires_utc_clock::now();
        ttlet tmp_tsc2 = time_stamp_count::now();

        if (tmp_tsc1.id() != tmp_tsc2.id()) {
            tt_log_fatal("CPU Switch detected during get_sample(), which should never happen");
        }

        if (tmp_tsc1.count() > tmp_tsc2.count()) {
            tt_log_warning("TSC skipped backwards");
            continue;
        }

        ttlet diff = tmp_tsc2.count() - tmp_tsc1.count();

        if (diff < shortest_diff) {
            shortest_diff = diff;
            shortest_tp = tmp_tp;
            shortest_tsc = {tmp_tsc1.count() + (diff / 2), tmp_tsc1.id()};
        }
    }

    if (shortest_diff == std::numeric_limits<uint64_t>::max()) {
        tt_log_fatal("Unable to get TSC sample.");
    }

    tsc = shortest_tsc;
    return shortest_tp;
}

[[nodiscard]] size_t hires_utc_clock::find_cpu_id(uint32_t cpu_id) noexcept
{
    auto cpu_id_ = _mm256_set1_epi32(cpu_id);

    ttlet num_calibrations_ = num_calibrations.load(std::memory_order_acquire);
    tt_axiom(num_calibrations_ < cpu_ids.size());

    // XXX We need to limit nr of CPUs that are calibrated.
    for (size_t i = 0; i < num_calibrations_; i += 8) {
        auto row = _mm256_loadu_si256(reinterpret_cast<__m256i const *>(cpu_ids.data() + i));
        auto row_result = _mm256_cmpeq_epi32(row, cpu_id_);
        auto row_result_ = _mm256_castsi256_ps(row_result);
        auto row_result_mask = _mm256_movemask_ps(row_result_);
        if (static_cast<bool>(row_result_mask)) {
            return i + std::countr_zero(static_cast<unsigned int>(row_result_mask));
        }
    }
    return cpu_ids.size();
}

[[nodiscard]] hires_utc_clock::time_point hires_utc_clock::make(time_stamp_count const &tsc) noexcept
{
    auto i = find_cpu_id(tsc.id());
    if (i != calibrations.size()) [[likely]] {
        ttlet &calibration = calibrations[i];
        ttlet tsc_epoch = calibration.tsc_epoch.load(std::memory_order::release);
        ttlet tsc_epoch_ = hires_utc_clock::time_point{std::chrono::nanoseconds{tsc_epoch}};
        return tsc_epoch_ + tsc.nanoseconds();
    }

    // Fallback.
    ttlet ref_tp = hires_utc_clock::now();
    ttlet ref_tsc = time_stamp_count::now();
    ttlet diff_ns = ref_tsc.nanoseconds() - tsc.nanoseconds();
    return ref_tp - diff_ns;
}

void hires_utc_clock::subsystem_proc(std::stop_token stop_token) noexcept
{
    set_thread_name("hires_utc_clock");

    // Calibrate the TSC frequency to within 1 ppm.
    // A 1s measurement already brings is to about 1ppm. We are
    // going to be taking average of the IQR of 11 samples, just
    // in case there are UTC clock adjustment made during the measurement.

    std::array<uint64_t,16> frequencies;
    for (auto i = 0; i != frequencies.size();) {
        ttlet f = time_stamp_count::measure_frequency(1s);
        if (f != 0) {
            frequencies[i] = f;
            ++i;
        }
    }
    std::ranges::sort(frequencies);
    ttlet iqr_size = frequencies.size() / 2;
    ttlet iqr_first = std::next(frequencies.cbegin(), frequencies.size() / 4);
    ttlet iqr_last = std::next(iqr_first, iqr_size);
    ttlet frequency = std::accumulate(iqr_first, iqr_last, uint64_t{0}) / iqr_size;

    tt_log_info("Accurate measurement of TSC frequency result is {} Hz", frequency);
    time_stamp_count::set_frequency(frequency);
}

[[nodiscard]] bool hires_utc_clock::init_subsystem() noexcept
{
    hires_utc_clock::subsystem_thread = std::jthread{subsystem_proc};
    return true;
}

void hires_utc_clock::deinit_subsystem() noexcept {
    if (hires_utc_clock::subsystem_thread.joinable()) {
        hires_utc_clock::subsystem_thread.request_stop();
        hires_utc_clock::subsystem_thread.join();
    }
}

bool hires_utc_clock::start_subsystem() noexcept
{
    return tt::start_subsystem(
        hires_utc_clock::subsystem_is_running, false, hires_utc_clock::init_subsystem, hires_utc_clock::deinit_subsystem);
}

void hires_utc_clock::stop_subsystem() noexcept
{
    return tt::stop_subsystem(hires_utc_clock::subsystem_is_running, false, hires_utc_clock::deinit_subsystem);
}

} // namespace tt
