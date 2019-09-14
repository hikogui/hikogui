// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

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
    std::function<void(void *)> callback;
    void *callbackData;

    void openAdapter() noexcept;
    void closeAdapter() noexcept;
    void wait() noexcept;

    static void verticalSyncThread(VerticalSync_win32 *self) noexcept;

public:
    VerticalSync_win32(std::function<void(void *)> callback, void *callbackData) noexcept;
    ~VerticalSync_win32();
};

}