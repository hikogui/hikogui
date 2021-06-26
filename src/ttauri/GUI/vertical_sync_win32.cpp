// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "vertical_sync_win32.hpp"
#include "../logger.hpp"
#include "../strings.hpp"
#include "../thread.hpp"
#include "../hires_utc_clock.hpp"
#include "../trace.hpp"
#include <Windows.h>
#undef WIN32_NO_STATUS
#include <ntstatus.h>
#include <numeric>

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

vertical_sync_win32::vertical_sync_win32() noexcept :
    vertical_sync()
{
    //Initialize driver hooks for D3DKMT.
    _gdi = LoadLibraryW(L"Gdi32.dll");
    if (!_gdi) {
        tt_log_fatal("Error opening Gdi32.dll {}", get_last_error_message());
    }

    // Grab the necessary function pointers needed to assist us in detecting vertical blank interrupts.
    pfnD3DKMTWaitForVerticalBlankEvent = (PFND3DKMT_WAITFORVERTICALBLANKEVENT) GetProcAddress(reinterpret_cast<HMODULE>(_gdi), "D3DKMTWaitForVerticalBlankEvent");
    pfnD3DKMTOpenAdapterFromHdc = (PFND3DKMT_OPENADAPTERFROMHDC)GetProcAddress(reinterpret_cast<HMODULE>(_gdi), "D3DKMTOpenAdapterFromHdc");
    pfnD3DKMTCloseAdapter = (PFND3DKMT_CLOSEADAPTER)GetProcAddress(reinterpret_cast<HMODULE>(_gdi), "D3DKMTCloseAdapter");

    if (!pfnD3DKMTOpenAdapterFromHdc) {
        tt_log_fatal("Error locating function D3DKMTOpenAdapterFromHdc");
    }

    if (!pfnD3DKMTCloseAdapter) {
        tt_log_fatal("Error locating function D3DKMTCloseAdapter");
    }

    if (!pfnD3DKMTWaitForVerticalBlankEvent) {
        tt_log_fatal("Error locating function D3DKMTWaitForVerticalBlankEvent!");
    }

    open_adapter();
}

vertical_sync_win32::~vertical_sync_win32()
{
    if (_adapter) {
        close_adapter();
    }
    FreeLibrary(reinterpret_cast<HMODULE>(_gdi));
}

void vertical_sync_win32::open_adapter() noexcept
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
        return;
    }

    D3DKMT_OPENADAPTERFROMHDC oa;
    memset(&oa, 0, sizeof(oa));
    oa.hDc = hdc;

    NTSTATUS status = pfnD3DKMTOpenAdapterFromHdc(&oa);
    if (status != STATUS_SUCCESS) {
        tt_log_error("Could not open adapter.");

    } else {
        _adapter = oa.hAdapter;
        _video_present_source_id = oa.VidPnSourceId;
    }
}

void vertical_sync_win32::close_adapter() noexcept
{
    D3DKMT_CLOSEADAPTER ca;

    ca.hAdapter = _adapter;

    NTSTATUS status = pfnD3DKMTCloseAdapter(&ca);
    if (status != STATUS_SUCCESS) {
        tt_log_error("Could not close adapter '{}'.", get_last_error_message());
    }

    _adapter = 0;
}

hires_utc_clock::duration vertical_sync_win32::average_frame_duration(hires_utc_clock::time_point frame_time_point) noexcept 
{
    ttlet currentDuration = _frame_duration_counter == 0 ? 16ms : frame_time_point - _previous_frame_time_point;
    _previous_frame_time_point = frame_time_point;

    _frame_duration_data[_frame_duration_counter++ % _frame_duration_data.size()] = currentDuration;

    ttlet number_of_elements = std::min(_frame_duration_counter, _frame_duration_data.size());
    ttlet last_i = _frame_duration_data.cbegin() + number_of_elements;
    ttlet sum = std::reduce(_frame_duration_data.cbegin(), last_i);
    return sum / number_of_elements;
}

hires_utc_clock::time_point vertical_sync_win32::wait() noexcept
{
    ttlet t = trace<"vertical_sync">();

    if (_adapter) {
        D3DKMT_WAITFORVERTICALBLANKEVENT we;

        we.hAdapter = _adapter;
        we.hDevice = 0;
        we.VidPnSourceId = _video_present_source_id;

        NTSTATUS status = pfnD3DKMTWaitForVerticalBlankEvent(&we);
        switch (status) {
        case STATUS_SUCCESS:
            break;
        case STATUS_DEVICE_REMOVED:
            tt_log_warning("gfx_device for vertical sync removed.");
            close_adapter();
            break;
        default:
            tt_log_error("Failed waiting for vertical sync. '{}'", get_last_error_message());
            close_adapter();
            break;
        }
    }

    if (not _adapter) {
        std::this_thread::sleep_for(16ms);
    }

    ttlet now = hires_utc_clock::now();

    return now + average_frame_duration(now);
}

}
