// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "statistics.hpp"
#include "logger.hpp"
#include "counters.hpp"
#include "trace.hpp"

namespace tt {

std::jthread statistics_thread;

static void statistics_flush_counters() noexcept
{
    ttlet keys = counter_map.keys();
    tt_log_statistics("{:>18} {:>9} {:>10} {:>10}", "total", "delta", "mean", "peak");
    for (ttlet &tag : keys) {
        ttlet[count, count_since_last_read] = read_counter(tag);
        tt_log_statistics("{:>18} {:>+9} {:10} {:10} {}", count, count_since_last_read, "", "", tag);
    }
}

static void statistics_flush_traces() noexcept
{
    ttlet keys = trace_statistics_map.keys();
    for (ttlet &tag : keys) {
        auto *stat = trace_statistics_map.get(tag, nullptr);
        tt_assert(stat != nullptr);
        ttlet stat_result = stat->read();

        if (stat_result.last_count <= 0) {
            tt_log_statistics("{:18d} {:+9d} {:10} {:10} {}", stat_result.count, stat_result.last_count, "", "", tag);

        } else {
            // XXX not perfect at all.
            ttlet duration_per_iter = format_engineering(stat_result.last_duration / stat_result.last_count);
            ttlet duration_peak = format_engineering(stat_result.peak_duration);
            tt_log_statistics(
                "{:18d} {:+9d} {:>10} {:>10} {}",
                stat_result.count,
                stat_result.last_count,
                duration_per_iter,
                duration_peak,
                tag);
        }
    }
}

static void statistics_flush() noexcept
{
    statistics_flush_counters();
    statistics_flush_traces();
}

static void statistics_loop(std::stop_token stop_token) noexcept
{
    set_thread_name("statistics");

    auto next_time = std::chrono::ceil<std::chrono::minutes>(hires_utc_clock::now());

    while (!stop_token.stop_requested()) {
        auto current_time = hires_utc_clock::now();
        if (current_time >= next_time) {
            statistics_flush();
            next_time = std::chrono::ceil<std::chrono::minutes>(current_time + 1s);
        }

        std::this_thread::sleep_for(100ms);
    }
}

void statistics_deinit() noexcept
{
    if (statistics_thread.joinable()) {
        statistics_thread.request_stop();
        statistics_thread.join();
    }
    statistics_flush();
}

void statistics_init() noexcept
{
    statistics_thread = std::jthread(statistics_loop);
}

}
