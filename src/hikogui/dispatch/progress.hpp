// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <memory>
#include <atomic>

hi_export_module(hikogui.dispatch : progress);

hi_export namespace hi { inline namespace v1 {

namespace detail {

class progress_data {
public:
    progress_data() noexcept = default;

    [[nodiscard]] bool stop_requested() const noexcept
    {
        return _stop_requested.load(std::memory_order::relaxed);
    }

    void request_stop() noexcept
    {
        _stop_requested.store(true, std::memory_order::relaxed);
    }

    /** Set the range of progress.
     *
     * @param first The start of the range.
     * @param last The end of the range.
     * @param name The name of the first item to work on.
     */
    void set_range(double first, double last, std::string name = std::string{})
    {
        auto const lock = std::scoped_lock(_mutex);
        _first = first;
        _last = last;
        _name = std::move(name);
    }

    void set_range(double last, std::string name = std::string{})
    {
        return set_total(0.0, last, std::move(name));
    }

    /** Set the current position within the range.
     *
     * @pre `set_range()` should be called first.
     * @pre value must be between first and last.
     * @param value The value of progress.
     * @param name The name of the next item to work on.
     */
    void set(double value, std::string name = std::string{})
    {
        auto const lock = std::scoped_lock(_mutex);
        _value = value;
        _name = std::move(name);
    }

    struct info_type {
        double first;
        double last;
        double value;
        std::string name;
        double ratio;
        double sub_ratio;
        double total_duration;
        double done_duration;
        double todo_duration;
    };

    [[nodiscard]] info_type info() const noexcept
    {
        auto r = info_type{};
        

        return r;
    }


    std::optional<float> progress() const noexcept
    {
        auto const r = _progress.load(std::memory_order::relaxed);
        if (r >= 0.0f and r <= 1.0f) {
            return r;
        } else {
            return std::nullopt;
        }
    }

    void set_progress(float rhs) noexcept
    {
        _progress.store(rhs, std::memory_order::relaxed);
    }

    std::chrono::duration current_duration() const noexcept
    {
        return std::chrono::utc_clock::now() - _start;
    }

    std::optional<std::chrono::duration> predicted_total_duration() const noexcept
    {
        if (_progress <= 0.0f) {
            return std::nullopt;
        } else {
            return current_duration() / _progress;
        }
    }

    std::optional<std::chrono::duration> predicted_duration_left() const noexcept
    {
        if (auto total_duration = predicted_total_duration()) {
            return total_duration * (1.0f - _progress);
        } else {
            return std::nullopt;
        }
    }


private:
    mutable std::mutex _mutex;
    bool _stop_requested = false;
    float _progress = 0.0f;

    utc_nanoseconds _start = {};
};

}

/** A object passed to a task for reporting progress.
 */
class progress_token {
public:
    progress_token(std::shared_ptr<detail::progress_data> data) : _data(std::move(data)) {}

    [[nodiscard]] bool stop_requested()
    {
        hi_axiom_not_null(_data);
        return _data->stop_requested();
    }

private:
    std::shared_ptr<detail::progress_data> _data;
};

class progress_source {
public:
    progress_source() : _data(std::make_shared<detail::progress_data>()) {}

    [[nodiscard]] progress_token get_token()
    {
        return {_data};
    }

    void request_stop() noexcept
    {
        hi_axiom_not_null(_data);
        return _data->request_stop();
    }

private:
    std::shared_ptr<detail::progress_data> _data;
};

}}
