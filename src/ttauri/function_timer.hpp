// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include "chrono.hpp"
#include <vector>
#include <algorithm>
#include <chrono>
#include <functional>

namespace tt::inline v1 {

/** A time that calls functions.
 *
 * @tparam Proto the prototype of the function passed.
 * @tparam Size the size of the function object.
 */
template<typename Proto = void()>
class function_timer {
public:
    using function_type = std::function<Proto>;
    using result_type = function_type::result_type;

    constexpr function_timer() noexcept = default;

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return _functions.empty();
    }

    /** Add a function to be called at a certain time.
     *
     * @param time_point The time when to call the function.
     * @param func The function to be called.
     */
    bool delay_function(utc_nanoseconds time_point, auto&& func) noexcept
    {
        ttlet it = std::lower_bound(_functions.begin(), _functions.end(), time_point, [](ttlet& x, ttlet& time_point) {
            return x.time_point > time_point;
        });

        ttlet next_to_call = it == _functions.end();

        _functions.emplace(it, time_point, std::chrono::nanoseconds::max(), tt_forward(func));
        return next_to_call;
    }

    /** Add a function to be called repeatedly.
     *
     * @param period The period between repeated calls
     * @param time_point The time when to call the function the first time.
     * @param func The function to be called.
     */
    bool repeat_function(std::chrono::nanoseconds period, utc_nanoseconds time_point, auto&& func) noexcept
    {
        auto it = std::lower_bound(_functions.begin(), _functions.end(), time_point, [](ttlet& x, ttlet& time_point) {
            return x.time_point > time_point;
        });

        it = _functions.emplace(it, time_point, period, tt_forward(func));
        return it + 1 == _functions.end();
    }

    /** Add a function to be called repeatedly.
     *
     * @param period The period between repeated calls
     * @param time_point The time when to call the function the first time.
     * @param func The function to be called.
     */
    bool repeat_function(std::chrono::nanoseconds period, auto&& func) noexcept
    {
        return repeat_function(period, std::chrono::utc_clock::now(), tt_forward(func));
    }

    /** Get the deadline of the next function to call.
     *
     * @return The deadline of the next function to call, or far/max into the future.
     */
    utc_nanoseconds current_deadline() const noexcept
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
    void run_all(utc_nanoseconds current_time, auto&&...args) noexcept
    {
        while (current_deadline() <= current_time) {
            run_one(current_time, args...);
        }
    }

private:
    struct timer_type {
        utc_nanoseconds time_point;
        std::chrono::nanoseconds period;
        function_type function;

        timer_type() noexcept = default;
        timer_type(timer_type const&) noexcept = default;
        timer_type(timer_type&&) noexcept = default;
        timer_type& operator=(timer_type const&) noexcept = default;
        timer_type& operator=(timer_type&&) noexcept = default;

        timer_type(utc_nanoseconds time_point, std::chrono::nanoseconds period, auto&& func) noexcept :
            time_point(time_point), period(period), function(tt_forward(func))
        {
        }

        timer_type(utc_nanoseconds time_point, auto&& func) noexcept :
            timer_type(time_point, std::chrono::nanoseconds::max(), tt_forward(func))
        {
        }

        timer_type(std::chrono::nanoseconds period, auto&& func) noexcept :
            timer_type(std::chrono::utc_clock::now(), period, tt_forward(func))
        {
        }

        [[nodiscard]] constexpr bool repeats() const noexcept
        {
            return period != std::chrono::nanoseconds::max();
        }
    };

    void remove_or_reinsert(utc_nanoseconds current_time) noexcept
    {
        tt_axiom(not _functions.empty());

        if (_functions.back().repeats()) {
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
            ttlet it = std::lower_bound(_functions.begin(), _functions.end(), item.time_point, [](ttlet& x, ttlet& time_point) {
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
     * @param args The arguments to pass to the function.
     * @return The return value of the function.
     */
    result_type run_one(utc_nanoseconds current_time, auto&&...args)
    {
        tt_axiom(not _functions.empty());

        if constexpr (std::is_same_v<result_type, void>) {
            _functions.back().function(tt_forward(args)...);
            remove_or_reinsert(current_time);
            return;
        } else {
            auto result = _functions.back().function(tt_forward(args)...);
            remove_or_reinsert(current_time);
            return result;
        }
    }

    /** Functions, sorted by descending time.
     */
    std::vector<timer_type> _functions;
};

} // namespace tt::inline v1
