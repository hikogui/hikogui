// Copyright 2020 Pokitec
// All rights reserved.

#include "timer.hpp"
#include "cpu_utc_clock.hpp"
#include "thread.hpp"
#include "logger.hpp"
#include <algorithm>

namespace tt {

[[nodiscard]] timer::time_point timer::calculate_next_wakeup(timer::time_point current_time, timer::duration interval) noexcept
{
    ttlet current_time_ = numeric_cast<int64_t>(current_time.time_since_epoch().count());
    ttlet interval_ = numeric_cast<int64_t>(interval.count());

    ttlet intervals_since_epoch = current_time_ / interval_;

    ttlet next_wakeup_ = (intervals_since_epoch + 1) * interval_;
    ttlet next_wakeup = timer::time_point{timer::duration{next_wakeup_}};

    tt_assume(next_wakeup <= current_time + interval);
    return next_wakeup;
}

timer::timer(std::string name) noexcept :
    name(std::move(name))
{
}

timer::~timer()
{
    stop();
    tt_assert(ssize(callback_list) == 0);
}

void timer::start_with_lock_held() noexcept
{
    stop_thread = false;
    thread = std::thread([this]() {
        set_thread_name(name);
        return loop();
    });
}

void timer::stop_with_lock_held() noexcept
{
    stop_thread = true;
    if (thread.joinable()) {
        auto tmp = std::thread{};
        std::swap(tmp, thread);

        mutex.unlock();
        tmp.join();
        mutex.lock();
    }
}

void timer::start() noexcept
{
    ttlet lock = std::scoped_lock(mutex);
    start_with_lock_held();
}

void timer::stop() noexcept
{
    ttlet lock = std::scoped_lock(mutex);
    stop_with_lock_held();
}

[[nodiscard]] std::pair<std::vector<timer::callback_type>,timer::time_point> timer::find_triggered_callbacks(
    timer::time_point current_time
) noexcept {
    ttlet lock = std::scoped_lock(mutex);

    auto triggered_callbacks = std::vector<callback_type>{};
    auto next_wakeup = timer::time_point::max();
    for (auto &&item: callback_list) {
        if (item.next_wakeup <= current_time) {
            triggered_callbacks.push_back(item.callback);
            item.next_wakeup = calculate_next_wakeup(current_time, item.interval);
        }

        // Protection against clock_settime().
        if (item.next_wakeup > current_time + item.interval) {
            item.next_wakeup = calculate_next_wakeup(current_time, item.interval);
        }

        if (item.next_wakeup < next_wakeup) {
            next_wakeup = item.next_wakeup;
        }
    }
    return {triggered_callbacks, next_wakeup};
}

void timer::loop() noexcept
{
    LOG_INFO("Timer {}: started", name);
    while (true) {
        ttlet current_time = hires_utc_clock::now();

        ttlet &[triggered_callbacks, next_wakeup] = find_triggered_callbacks(current_time);

        // Execute all the triggered callbacks.
        for (ttlet &callback: triggered_callbacks) {
            callback(current_time, false);
        }

        ttlet sleep_duration = std::min(next_wakeup - current_time, timer::duration{100ms});
        if (sleep_duration > 0ms) {
            std::this_thread::sleep_for(sleep_duration);
        }

        ttlet lock = std::scoped_lock(mutex);
        if (stop_thread || ssize(callback_list) == 0) {
            break;
        }
    }
    LOG_INFO("Timer {}: finishing up", name);
    {
        ttlet current_time = hires_utc_clock::now();
        ttlet lock = std::scoped_lock(mutex);
        for (ttlet &item: callback_list) {
            item.callback(current_time, true);
        }
    }
    LOG_INFO("Timer {}: finished", name);
}

void timer::remove_callback(size_t callback_id) noexcept
{
    ttlet lock = std::scoped_lock(mutex);

    ttlet i = std::find_if(callback_list.cbegin(), callback_list.cend(), [callback_id](ttlet &item) {
        return item.id == callback_id;
    });
    tt_assert(i != callback_list.cend());

    callback_list.erase(i);

    if (ssize(callback_list) == 0) {
        stop_with_lock_held();
    }
}

}
