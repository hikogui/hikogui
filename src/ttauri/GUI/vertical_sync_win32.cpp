// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "vertical_sync_win32.hpp"
#include "../logger.hpp"
#include "../strings.hpp"
#include "../thread.hpp"
#include "../cpu_utc_clock.hpp"
#include <Windows.h>
#undef WIN32_NO_STATUS
#include <ntstatus.h>
#include <algorithm>

typedef UINT D3DKMT_HANDLE;
typedef UINT D3DDDI_VIDEO_PRESENT_SOURCE_ID;
typedef UINT D3DDDI_VIDEO_PRESENT_TARGET_ID;

typedef struct _D3DKMT_OPENADAPTERFROMHDC {
    HDC                             hDc;            // in:  DC that maps to a single display
    D3DKMT_HANDLE                   hAdapter;       // out: adapter handle
    LUID                            AdapterLuid;    // out: adapter LUID
    D3DDDI_VIDEO_PRESENT_SOURCE_ID  VidPnSourceId;  // out: VidPN source ID for that particular display
} D3DKMT_OPENADAPTERFROMHDC;

typedef struct _D3DKMT_CLOSEADAPTER {
    D3DKMT_HANDLE hAdapter;
} D3DKMT_CLOSEADAPTER;

typedef struct _D3DKMT_WAITFORVERTICALBLANKEVENT {
    D3DKMT_HANDLE                   hAdapter;      // in: adapter handle
    D3DKMT_HANDLE                   hDevice;       // in: device handle [Optional]
    D3DDDI_VIDEO_PRESENT_SOURCE_ID  VidPnSourceId; // in: adapter's VidPN Source ID
} D3DKMT_WAITFORVERTICALBLANKEVENT;

typedef NTSTATUS(APIENTRY* PFND3DKMT_WAITFORVERTICALBLANKEVENT)(_In_ CONST D3DKMT_WAITFORVERTICALBLANKEVENT*);
typedef NTSTATUS(APIENTRY* PFND3DKMT_OPENADAPTERFROMHDC)(_Inout_ D3DKMT_OPENADAPTERFROMHDC*);
typedef NTSTATUS(APIENTRY* PFND3DKMT_CLOSEADAPTER)(_Inout_ D3DKMT_CLOSEADAPTER*);

PFND3DKMT_WAITFORVERTICALBLANKEVENT pfnD3DKMTWaitForVerticalBlankEvent;
PFND3DKMT_OPENADAPTERFROMHDC		pfnD3DKMTOpenAdapterFromHdc;
PFND3DKMT_CLOSEADAPTER		        pfnD3DKMTCloseAdapter;

namespace tt {

using namespace std;

vertical_sync_win32::vertical_sync_win32(std::function<void(void*,hires_utc_clock::time_point)> callback, void* callbackData) noexcept :
    vertical_sync_base(callback, callbackData)
{
    state = State::ADAPTER_CLOSED;

    /*
     * Initialize driver hooks for D3DKMT.
     *
     * Grab the necessary function pointers needed to assist us in detecting vertical blank interrupts.
     */

    gdi = LoadLibraryW(L"Gdi32.dll");
    if (!gdi) {
        tt_log_fatal("Error opening Gdi32.dll {}", get_last_error_message());
    }

    pfnD3DKMTWaitForVerticalBlankEvent = (PFND3DKMT_WAITFORVERTICALBLANKEVENT) GetProcAddress(reinterpret_cast<HMODULE>(gdi), "D3DKMTWaitForVerticalBlankEvent");
    pfnD3DKMTOpenAdapterFromHdc = (PFND3DKMT_OPENADAPTERFROMHDC)GetProcAddress(reinterpret_cast<HMODULE>(gdi), "D3DKMTOpenAdapterFromHdc");
    pfnD3DKMTCloseAdapter = (PFND3DKMT_CLOSEADAPTER)GetProcAddress(reinterpret_cast<HMODULE>(gdi), "D3DKMTCloseAdapter");

    if (!pfnD3DKMTOpenAdapterFromHdc) {
        tt_log_fatal("Error locating function D3DKMTOpenAdapterFromHdc");
    }

    if (!pfnD3DKMTCloseAdapter) {
        tt_log_fatal("Error locating function D3DKMTCloseAdapter");
    }

    if (!pfnD3DKMTWaitForVerticalBlankEvent) {
        tt_log_fatal("Error locating function D3DKMTWaitForVerticalBlankEvent!");
    }

    verticalSyncThreadID = std::thread([=]() {
        set_thread_name("vertical_sync");
        tt_log_info("Started: vertical-sync thread.");
        this->verticalSyncThread();
        tt_log_info("Finished: vertical-sync thread.");
    });
}

vertical_sync_win32::~vertical_sync_win32() {
    stop = true;
    verticalSyncThreadID.join();

    FreeLibrary(reinterpret_cast<HMODULE>(gdi));
}

void vertical_sync_win32::openAdapter() noexcept
{
    // Search for primary display device.
    DISPLAY_DEVICEW dd;
    memset(&dd, 0, sizeof(dd));
    dd.cb = sizeof(dd);
    for (int i = 0; EnumDisplayDevicesW(NULL, i, &dd, 0); ++i) {
        if (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) {
            break;
        }
    }

    tt_log_info("Found primary display device '{}'.", to_string(wstring(dd.DeviceName)));

    HDC hdc = CreateDCW(NULL, dd.DeviceName, NULL, NULL);
    if (hdc == NULL) {
        tt_log_error("Could not get handle to primary display device.");
        state = State::FALLBACK;
        return;
    }

    D3DKMT_OPENADAPTERFROMHDC oa;
    memset(&oa, 0, sizeof(oa));
    oa.hDc = hdc;

    NTSTATUS status = pfnD3DKMTOpenAdapterFromHdc(&oa);
    if (status != STATUS_SUCCESS) {
        tt_log_error("Could not open adapter.");
        state = State::FALLBACK;

    } else {
        adapter = oa.hAdapter;
        videoPresentSourceID = oa.VidPnSourceId;
        state = State::ADAPTER_OPEN;
    }
}

void vertical_sync_win32::closeAdapter() noexcept
{
    D3DKMT_CLOSEADAPTER ca;

    ca.hAdapter = adapter;

    NTSTATUS status = pfnD3DKMTCloseAdapter(&ca);
    if (status != STATUS_SUCCESS) {
        tt_log_error("Could not close adapter '{}'.", get_last_error_message());
        state = State::FALLBACK;
    } else {
        state = State::ADAPTER_CLOSED;
    }
}

hires_utc_clock::duration vertical_sync_win32::averageFrameDuration(hires_utc_clock::time_point frameTimestamp) noexcept 
{
    ttlet currentDuration = frameDurationDataCounter == 0 ? 16ms : frameTimestamp - previousFrameTimestamp;
    previousFrameTimestamp = frameTimestamp;

    frameDurationData[frameDurationDataCounter++ % frameDurationData.size()] = currentDuration;

    ttlet number_of_elements = std::min(frameDurationDataCounter, frameDurationData.size());
    ttlet last_i = frameDurationData.cbegin() + number_of_elements;
    ttlet sum = std::reduce(frameDurationData.cbegin(), last_i);
    return sum / number_of_elements;
}

hires_utc_clock::time_point vertical_sync_win32::wait() noexcept
{
    if (state == State::ADAPTER_CLOSED) {
        openAdapter();
    }

    if (state == State::ADAPTER_OPEN) {
        D3DKMT_WAITFORVERTICALBLANKEVENT we;

        we.hAdapter = adapter;
        we.hDevice = 0;
        we.VidPnSourceId = videoPresentSourceID;

        NTSTATUS status = pfnD3DKMTWaitForVerticalBlankEvent(&we);
        switch (status) {
        case STATUS_SUCCESS:
            break;
        case STATUS_DEVICE_REMOVED:
            tt_log_warning("gui_device for vertical sync removed.");
            closeAdapter();
            break;
        default:
            tt_log_error("Failed waiting for vertical sync. '{}'", get_last_error_message());
            closeAdapter();
            state = State::FALLBACK;
            break;
        }
    }

    if (state != State::ADAPTER_OPEN) {
        std::this_thread::sleep_for(16ms);
    }

    ttlet now = cpu_utc_clock::now();

    return now + averageFrameDuration(now);
}

void vertical_sync_win32::verticalSyncThread() noexcept
{
    while (!stop) {
        ttlet displayTimePoint = wait();
        callback(callbackData, displayTimePoint);
    }
}


}
