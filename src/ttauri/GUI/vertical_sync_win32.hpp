// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "vertical_sync_base.hpp"
#include <span>
#include <thread>
#include <array>

namespace tt {

class vertical_sync_win32 final : public vertical_sync_base {
    enum class State {
        ADAPTER_OPEN,
        ADAPTER_CLOSED,
        FALLBACK
    };

    State state;

    void *gdi;
    unsigned int adapter;
    unsigned int videoPresentSourceID;

    std::thread verticalSyncThreadID;
    bool stop = false;

    hires_utc_clock::time_point previousFrameTimestamp;
    std::array<hires_utc_clock::duration,15> frameDurationData;
    size_t frameDurationDataCounter = 0;

    void openAdapter() noexcept;
    void closeAdapter() noexcept;

    /** Returns the median duration between two frames.
     */
    [[nodiscard]] hires_utc_clock::duration averageFrameDuration(hires_utc_clock::time_point frameTimestamp) noexcept;

    /** Waits for vertical-sync
     * @return Timestamp when the current frame will be displayed.
     */
    [[nodiscard]] hires_utc_clock::time_point wait() noexcept;

    void verticalSyncThread() noexcept;

public:
    vertical_sync_win32(std::function<void(void *,hires_utc_clock::time_point)> callback, void *callbackData) noexcept;
    ~vertical_sync_win32();
};

}
