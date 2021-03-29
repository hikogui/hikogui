// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "timer.hpp"
#include "thread.hpp"
#include "logger.hpp"
#include <algorithm>

namespace tt {

[[nodiscard]] timer::time_point timer::calculate_next_wakeup(timer::time_point current_time, timer::duration interval) noexcept
{
    ttlet current_time_ = narrow_cast<int64_t>(current_time.time_since_epoch().count());
    ttlet interval_ = narrow_cast<int64_t>(interval.count());

    ttlet intervals_since_epoch = current_time_ / interval_;

    ttlet next_wakeup_ = (intervals_since_epoch + 1) * interval_;
    ttlet next_wakeup = timer::time_point{timer::duration{next_wakeup_}};

    tt_axiom(next_wakeup <= current_time + interval);
    return next_wakeup;
}

timer::timer(std::string name) noexcept :
    name(std::move(name))
{
}

timer::~timer()
{
    stop();
    tt_assert(std::ssize(callback_list) == 0);
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
    mutex.lock();
    start_with_lock_held();
    mutex.unlock();
}

void timer::stop() noexcept
{
    mutex.lock();
    stop_with_lock_held();
    mutex.unlock();
}

[[nodiscard]] std::pair<std::vector<timer::callback_ptr_type>,timer::time_point> timer::find_triggered_callbacks(
    timer::time_point current_time
) noexcept {
    ttlet lock = std::scoped_lock(mutex);

    auto triggered_callbacks = std::vector<callback_ptr_type>{};
    auto next_wakeup = timer::time_point::max();

    auto i = callback_list.begin();
    while (i != callback_list.end()) {
        auto callback_ptr = i->callback_ptr.lock();

        if (callback_ptr) {
            if (i->next_wakeup <= current_time) {
                triggered_callbacks.push_back(callback_ptr);
                i->next_wakeup = calculate_next_wakeup(current_time, i->interval);
            }

            // Protection against clock_settime().
            if (i->next_wakeup > current_time + i->interval) {
                i->next_wakeup = calculate_next_wakeup(current_time, i->interval);
            }

            if (i->next_wakeup < next_wakeup) {
                next_wakeup = i->next_wakeup;
            }

            ++i;
        } else {
            i = callback_list.erase(i);
        }

        
    }
    return {triggered_callbacks, next_wakeup};
}

void timer::loop() noexcept
{
    tt_log_info("Timer {}: started", name);
    while (true) {
        ttlet current_time = hires_utc_clock::now();

        ttlet &[triggered_callbacks, next_wakeup] = find_triggered_callbacks(current_time);

        // Execute all the triggered callbacks.
        for (ttlet &callback_ptr: triggered_callbacks) {
            (*callback_ptr)(current_time, false);
        }

        // Sleep, but not for more than 100ms.
        ttlet sleep_duration = std::min(next_wakeup - current_time, timer::duration{100ms});
        if (sleep_duration > 0ms) {
            std::this_thread::sleep_for(sleep_duration);
        }

        ttlet lock = std::scoped_lock(mutex);
        if (stop_thread || std::ssize(callback_list) == 0) {
            break;
        }
    }
    tt_log_info("Timer {}: finishing up", name);

    ttlet lock = std::scoped_lock(mutex);
    
    ttlet current_time = hires_utc_clock::now();
    for (ttlet &item: callback_list) {
        if (auto callback_ptr_ = item.callback_ptr.lock()) {
            (*callback_ptr_)(current_time, true);
        }
    }
    callback_list.clear();

    tt_log_info("Timer {}: finished", name);
}

void timer::remove_callback(callback_ptr_type const &callback_ptr) noexcept
{
    ttlet lock = std::scoped_lock(mutex);

    ttlet i = std::remove_if(callback_list.begin(), callback_list.end(), [&callback_ptr](ttlet &item) {
        auto callback_ptr_ = item.callback_ptr.lock();
        return !callback_ptr_ || callback_ptr_ == callback_ptr;
    });
    callback_list.erase(i, callback_list.end());
}

}
