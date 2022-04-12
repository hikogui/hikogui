// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "counters.hpp"
#include "log.hpp"

namespace tt::inline v1::detail {

void counter::log_header() noexcept
{
    tt_log_statistics("");
    tt_log_statistics("{:>18} {:>9} {:>10} {:>10} {:>10}", "total", "delta", "min", "max", "mean");
    tt_log_statistics("------------------ --------- ---------- ---------- ----------");
}

/** Log the counter.
 */
void counter::log(std::string const &tag) noexcept
{
    ttlet total_count = _total_count.load(std::memory_order::relaxed);
    ttlet prev_count = _prev_count.exchange(_total_count, std::memory_order::relaxed);
    ttlet duration_max = time_stamp_count::duration_from_count(_duration_max.exchange(0, std::memory_order::relaxed));
    ttlet duration_min = time_stamp_count::duration_from_count(
        _duration_min.exchange(std::numeric_limits<uint64_t>::max(), std::memory_order::relaxed));

    ttlet duration_avg = _duration_avg.exchange(0, std::memory_order::relaxed);
    if (duration_avg == 0) {
        tt_log_statistics("{:>18} {:>+9} {:10} {:10} {:10} {}", total_count, total_count - prev_count, "", "", "", tag);

    } else {
        ttlet avg_count = duration_avg & 0xffff;
        ttlet avg_sum = duration_avg >> 16;
        ttlet average = time_stamp_count::duration_from_count(avg_sum / avg_count);

        tt_log_statistics(
            "{:18d} {:+9d} {:>10} {:>10} {:>10} {}",
            total_count,
            total_count - prev_count,
            format_engineering(duration_min),
            format_engineering(duration_max),
            format_engineering(average),
            tag);
    }
}

} // namespace tt::inline v1::detail
