// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "vertical_sync.hpp"
#include <span>
#include <thread>
#include <array>

namespace tt {

class vertical_sync_win32 final : public vertical_sync {
public:
    vertical_sync_win32() noexcept;
    ~vertical_sync_win32() override;

    [[nodiscard]] hires_utc_clock::time_point wait() noexcept override;

private:
    void *_gdi;
    unsigned int _adapter = 0;
    unsigned int _video_present_source_id;

    hires_utc_clock::time_point _previous_frame_time_point;
    std::array<hires_utc_clock::duration, 15> _frame_duration_data;
    size_t _frame_duration_counter = 0;

    void open_adapter() noexcept;
    void close_adapter() noexcept;

    /** Returns the median duration between two frames.
     */
    [[nodiscard]] hires_utc_clock::duration average_frame_duration(hires_utc_clock::time_point frameTimestamp) noexcept;
};

}
