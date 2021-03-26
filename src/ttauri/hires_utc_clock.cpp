// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hires_utc_clock.hpp"
#include "time_stamp_count.hpp"
#include "application.hpp"
#include "logger.hpp"
#include <immintrin.h>
#include <fmt/ostream.h>
#include <fmt/format.h>
#include <bit>

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

    std::atomic_thread_fence(std::memory_order_acquire);
    // XXX We need to limit nr of CPUs that are calibrated.
    for (size_t i = 0; i < cpu_ids.size(); i += 8) {
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
    if (i < calibrations.size()) {
        [[likelly]] ttlet &calibration = calibrations[i];
        ttlet offset = calibration.offset.load(std::memory_order::release);
        if (offset != 0) {
            [[likelly]] ttlet offset_ = hires_utc_clock::time_point{std::chrono::nanoseconds{offset}};
            return offset_ + tsc.nanoseconds();
        }
    }
    // Fallback.
    ttlet ref_tp = hires_utc_clock::now();
    ttlet ref_tsc = time_stamp_count::now();
    ttlet diff_ns = ref_tsc.nanoseconds() - tsc.nanoseconds();
    return ref_tp - diff_ns;
}

} // namespace tt
