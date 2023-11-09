// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/utility.hpp"
#include "../time/time.hpp"
#include "../concurrency/concurrency.hpp"
#include "../macros.hpp"
#include <vector>
#include <algorithm>
#include <chrono>
#include <functional>

hi_export_module(hikogui.dispatch.function_timer);

hi_export namespace hi::inline v1 {

/** A timer that calls functions.
 */
class function_timer {
public:
    constexpr function_timer() noexcept = default;

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return _functions.empty();
    }

    /** Add a function to be called at a certain time.
     *
     * @param time_point The time when to call the function.
     * @param callback The function to be called.
     * @return token, next to call.
     */
    template<forward_of<void()> Func>
    [[nodiscard]] std::pair<callback<void()>, bool> delay_function(utc_nanoseconds time_point, Func &&func) noexcept
    {
        hilet it = std::lower_bound(_functions.begin(), _functions.end(), time_point, [](hilet& x, hilet& time_point) {
            return x.time_point > time_point;
        });

        hilet next_to_call = it == _functions.end();

        auto token = callback<void()>{std::forward<Func>(func)};
        _functions.emplace(it, time_point, std::chrono::nanoseconds::max(), token);
        return {std::move(token), next_to_call};
    }

    /** Add a function to be called repeatedly.
     *
     * @param period The period between repeated calls
     * @param time_point The time when to call the function the first time.
     * @param callback The function to be called.
     * @return token, next to call.
     */
    template<forward_of<void()> Func>
    [[nodiscard]] std::pair<callback<void()>, bool> repeat_function(
        std::chrono::nanoseconds period,
        utc_nanoseconds time_point,
        Func &&func) noexcept
    {
        auto it = std::lower_bound(_functions.begin(), _functions.end(), time_point, [](hilet& x, hilet& time_point) {
            return x.time_point > time_point;
        });

        auto token = callback<void()>{std::forward<Func>(func)};
        it = _functions.emplace(it, time_point, period, token);
        return {std::move(token), it + 1 == _functions.end()};
    }

    /** Add a function to be called repeatedly.
     *
     * @param period The period between repeated calls
     * @param callback The function to be called.
     * @return token, next to call.
     */
    template<forward_of<void()> Func>
    [[nodiscard]] std::pair<callback<void()>, bool> repeat_function(std::chrono::nanoseconds period, Func &&func) noexcept
    {
        return repeat_function(period, std::chrono::utc_clock::now(), std::forward<Func>(func));
    }

    /** Get the deadline of the next function to call.
     *
     * @return The deadline of the next function to call, or far/max into the future.
     */
    [[nodiscard]] utc_nanoseconds current_deadline() const noexcept
    {
        if (_functions.empty()) {
            return utc_nanoseconds::max();
        } else {
            return _functions.back().time_point;
        }
    }

    /** Run all the function that should have run by the current_time.
     *
     * @param current_time The current time.
     * @param args The arguments to pass to the function.
     */
    void run_all(utc_nanoseconds current_time, auto const&...args) noexcept
    {
        while (current_deadline() <= current_time) {
            run_one(current_time, args...);
        }
    }

private:
    struct timer_type {
        utc_nanoseconds time_point;
        std::chrono::nanoseconds period;
        weak_callback<void()> callback;

        timer_type() noexcept = default;
        timer_type(timer_type const&) noexcept = default;
        timer_type(timer_type&&) noexcept = default;
        timer_type& operator=(timer_type const&) noexcept = default;
        timer_type& operator=(timer_type&&) noexcept = default;

        timer_type(utc_nanoseconds time_point, std::chrono::nanoseconds period, weak_callback<void()> callback) noexcept :
            time_point(time_point), period(period), callback(std::move(callback))
        {
        }

        timer_type(utc_nanoseconds time_point, weak_callback<void()> callback) noexcept :
            timer_type(time_point, std::chrono::nanoseconds::max(), std::move(callback))
        {
        }

        timer_type(std::chrono::nanoseconds period, weak_callback<void()> callback) noexcept :
            timer_type(std::chrono::utc_clock::now(), period, std::move(callback))
        {
        }

        [[nodiscard]] constexpr bool repeats() const noexcept
        {
            return period != std::chrono::nanoseconds::max();
        }
    };

    void remove_or_reinsert(utc_nanoseconds current_time) noexcept
    {
        hi_assert(not _functions.empty());

        if (_functions.back().repeats() and not _functions.back().callback.expired()) {
            // When the function is repeating, calculate the new.
            auto item = std::move(_functions.back());
            _functions.pop_back();

            // Delay the function to be called on the next period.
            // However if the current_time already is passed the deadline, delay it even further.
            item.time_point += item.period;
            if (item.time_point > current_time) {
                item.time_point = current_time + item.period;
            }

            // Reinsert the function in the sorted list of functions.
            hilet it = std::lower_bound(_functions.begin(), _functions.end(), item.time_point, [](hilet& x, hilet& time_point) {
                return x.time_point > time_point;
            });

            _functions.insert(it, std::move(item));

        } else {
            _functions.pop_back();
        }
    }

    /** Call the next function on the list.
     *
     * @note it is undefined behavior to call this function if the current_deadline() has not passed.
     * @param current_time The current time, this is used when reinserting periodic function to handle starvation issues.
     */
    void run_one(utc_nanoseconds current_time)
    {
        hi_assert(not _functions.empty());

        auto &callback = _functions.back().callback;
        if (callback.lock()) {
            callback();
            callback.unlock();
        }
        remove_or_reinsert(current_time);
    }

    /** Functions, sorted by descending time.
     */
    std::vector<timer_type> _functions;
};

} // namespace hi::inline v1
