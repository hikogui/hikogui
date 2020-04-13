// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/cpu_utc_clock.hpp"
#include <gsl/gsl>
#include <thread>

namespace TTauri::GUI {

class VerticalSync_win32 {
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
    std::function<void(void *,cpu_utc_clock::time_point)> callback;
    void *callbackData;

    cpu_utc_clock::time_point previousFrameTimestamp;
    std::array<cpu_utc_clock::duration,15> frameDurationData;
    size_t frameDurationDataCounter = 0;

    void openAdapter() noexcept;
    void closeAdapter() noexcept;

    /** Returns the median duration between two frames.
     */
    [[nodiscard]] cpu_utc_clock::duration averageFrameDuration(cpu_utc_clock::time_point frameTimestamp) noexcept;

    /** Waits for vertical-sync
     * @return Timestamp when the current frame will be displayed.
     */
    [[nodiscard]] cpu_utc_clock::time_point wait() noexcept;

    void verticalSyncThread() noexcept;

public:
    VerticalSync_win32(std::function<void(void *,cpu_utc_clock::time_point)> callback, void *callbackData) noexcept;
    ~VerticalSync_win32();
};

}