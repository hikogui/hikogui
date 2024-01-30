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

private:
    std::atomic<bool> _stop_requested = false;
    std::atomic<float> _progress = -1.0f;
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
