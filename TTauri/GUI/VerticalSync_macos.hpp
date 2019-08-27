// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <gsl/gsl>
#include <thread>

namespace TTauri::GUI {

class VerticalSync_macos {
    enum class State {
        ADAPTER_OPEN,
        ADAPTER_CLOSED,
        FALLBACK
    };

    State state;

    std::thread verticalSyncThreadID;
    bool stop = false;
    std::function<void(void *)> callback;
    void *callbackData;

    void openAdapter() noexcept;
    void closeAdapter() noexcept;
    void wait() noexcept;

    static void verticalSyncThread(VerticalSync_macos *self) noexcept;

public:
    VerticalSync_macos(std::function<void(void *)> callback, void *callbackData) noexcept;
    ~VerticalSync_macos();
};

}
