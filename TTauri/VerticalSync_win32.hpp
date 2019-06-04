// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <Windows.h>
#include <gsl/gsl>
#include <thread>

typedef UINT D3DKMT_HANDLE;
typedef UINT D3DDDI_VIDEO_PRESENT_SOURCE_ID;

namespace TTauri {

class VerticalSync_win32 {
    enum class State {
        ADAPTER_OPEN,
        ADAPTER_CLOSED,
        FALLBACK
    };

    State state;

    HMODULE gdi;
    D3DKMT_HANDLE adapter;
    D3DDDI_VIDEO_PRESENT_SOURCE_ID videoPresentSourceID;

    std::thread verticalSyncThreadID;
    bool stop = false;
    std::function<void(void)> callback;

    void openAdapter();
    void closeAdapter();
    void wait();

    static void verticalSyncThread(VerticalSync_win32 *self);

public:
    VerticalSync_win32(std::function<void(void)> callback);
    ~VerticalSync_win32();
};

}