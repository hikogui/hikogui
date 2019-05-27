// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Window.hpp"
#include "TTauri/all.hpp"
#include <gsl/gsl>

#ifdef _WIN32
#include <Windows.h>

typedef UINT D3DKMT_HANDLE;
typedef UINT D3DDDI_VIDEO_PRESENT_SOURCE_ID;


#endif

namespace TTauri::GUI {

class VerticalSync {
    enum class State {
        ADAPTER_OPEN,
        ADAPTER_CLOSED,
        FALLBACK
    };

    State state;
    std::vector<std::weak_ptr<Window>> windows;

#ifdef _WIN32
    HMODULE gdi;
    D3DKMT_HANDLE adapter;
    D3DDDI_VIDEO_PRESENT_SOURCE_ID videoPresentSourceID;
    std::thread renderThreadID;
    bool stopRenderThread = false;

    static void renderThread(gsl::not_null<VerticalSync*> self);
    void openAdapter();
    void closeAdapter();
    void waitForVSync();
#endif

public:
    VerticalSync();
    ~VerticalSync();

    void add(std::weak_ptr<Window> window) {
        windows.push_back(std::move(window));
    }

    void cleanup() {
        erase_if(windows, [](auto const &x) {
            return x.expired();
        });
    }
};

}