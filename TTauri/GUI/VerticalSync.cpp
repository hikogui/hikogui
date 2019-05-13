
#include "VerticalSync.hpp"
#include "TTauri/logging.hpp"
#include "TTauri/strings.hpp"

#include <ntstatus.h>

#ifdef _WIN32
typedef UINT D3DDDI_VIDEO_PRESENT_TARGET_ID;

typedef struct _D3DKMT_OPENADAPTERFROMHDC
{
    HDC                             hDc;            // in:  DC that maps to a single display
    D3DKMT_HANDLE                   hAdapter;       // out: adapter handle
    LUID                            AdapterLuid;    // out: adapter LUID
    D3DDDI_VIDEO_PRESENT_SOURCE_ID  VidPnSourceId;  // out: VidPN source ID for that particular display
} D3DKMT_OPENADAPTERFROMHDC;

typedef struct _D3DKMT_CLOSEADAPTER {
    D3DKMT_HANDLE hAdapter;
} D3DKMT_CLOSEADAPTER;

typedef struct _D3DKMT_WAITFORVERTICALBLANKEVENT
{
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
#endif

namespace TTauri::GUI {



using namespace std;
using namespace gsl;

VerticalSync::VerticalSync() {
    state = VerticalSync::State::ADAPTER_CLOSED;

#ifdef _WIN32
    /*
     * Initialize driver hooks for D3DKMT.
     *
     * Grab the necessary function pointers needed to assist us in detecting vertical blank interrupts.
     */

    if (!(gdi = LoadLibraryW(L"Gdi32.dll"))) {
        LOG_FATAL("Error opening Gdi32.dll %s") << getLastErrorMessage();
        abort();
    }

    pfnD3DKMTWaitForVerticalBlankEvent = (PFND3DKMT_WAITFORVERTICALBLANKEVENT) GetProcAddress(gdi, "D3DKMTWaitForVerticalBlankEvent");
    pfnD3DKMTOpenAdapterFromHdc = (PFND3DKMT_OPENADAPTERFROMHDC)GetProcAddress(gdi, "D3DKMTOpenAdapterFromHdc");
    pfnD3DKMTCloseAdapter = (PFND3DKMT_CLOSEADAPTER)GetProcAddress(gdi, "D3DKMTCloseAdapter");

    if (!pfnD3DKMTOpenAdapterFromHdc) {
        LOG_FATAL("Error locating function D3DKMTOpenAdapterFromHdc");
        abort();
    }

    if (!pfnD3DKMTCloseAdapter) {
        LOG_FATAL("Error locating function D3DKMTCloseAdapter");
        abort();
    }

    if (!pfnD3DKMTWaitForVerticalBlankEvent) {
        LOG_FATAL("Error locating function D3DKMTWaitForVerticalBlankEvent!");
        abort();
    }

    renderThreadID = thread(renderThread, not_null<VerticalSync*>(this));

#endif
}

VerticalSync::~VerticalSync() {
#ifdef _WIN32
    stopRenderThread = true;
    renderThreadID.join();

    FreeLibrary(gdi);
#endif
}

#ifdef _WIN32
void VerticalSync::openAdapter()
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

    LOG_INFO("Found primary display device '%s'.") % translateString<string>(wstring(dd.DeviceName));

    HDC hdc = CreateDCW(NULL, dd.DeviceName, NULL, NULL);
    if (hdc == NULL) {
        LOG_ERROR("Could not get handle to primary display device.");
        state = VerticalSync::State::FALLBACK;
        return;
    }

    D3DKMT_OPENADAPTERFROMHDC oa;
    memset(&oa, 0, sizeof(oa));
    oa.hDc = hdc;

    NTSTATUS status = pfnD3DKMTOpenAdapterFromHdc(&oa);
    if (status != STATUS_SUCCESS) {
        LOG_ERROR("Could not open adapter.");
        state = VerticalSync::State::FALLBACK;

    } else {
        adapter = oa.hAdapter;
        videoPresentSourceID = oa.VidPnSourceId;
        state = VerticalSync::State::ADAPTER_OPEN;
    }
}

void VerticalSync::closeAdapter()
{
    D3DKMT_CLOSEADAPTER ca;

    ca.hAdapter = adapter;

    NTSTATUS status = pfnD3DKMTCloseAdapter(&ca);
    if (status != STATUS_SUCCESS) {
        LOG_ERROR("Could not close adapter '%s'.") % getLastErrorMessage();
        state = VerticalSync::State::FALLBACK;
    } else {
        state = VerticalSync::State::ADAPTER_CLOSED;
    }
}

void VerticalSync::waitForVSync()
{
    if (state == VerticalSync::State::ADAPTER_CLOSED) {
        openAdapter();
    }

    if (state == VerticalSync::State::ADAPTER_OPEN) {
        D3DKMT_WAITFORVERTICALBLANKEVENT we;

        we.hAdapter = adapter;
        we.hDevice = 0;
        we.VidPnSourceId = videoPresentSourceID;

        NTSTATUS status = pfnD3DKMTWaitForVerticalBlankEvent(&we);
        switch (status) {
        case STATUS_SUCCESS:
            break;
        case STATUS_DEVICE_REMOVED:
            LOG_WARNING("Device for vertical sync removed.");
            closeAdapter();
            break;
        default:
            LOG_ERROR("Failed waiting for vertical sync. '%s'") % getLastErrorMessage();
            closeAdapter();
            state = VerticalSync::State::FALLBACK;
            break;
        }
    }

    if (state != VerticalSync::State::ADAPTER_OPEN) {
        std::this_thread::sleep_for(16ms);
    }
}

void VerticalSync::renderThread(gsl::not_null<VerticalSync*> self)
{
    auto threadID = GetCurrentThread();
    SetThreadDescription(threadID, L"TTauri::GUI Update And Render");

    while (!self->stopRenderThread) {
        auto needsCleanup = false;

        self->waitForVSync();

        for (auto const &window: self->windows) {
            if (auto window_ = window.lock()) {
                window_->updateAndRender(0, 0);
            } else {
                needsCleanup = true;
            }
        }

        if (needsCleanup) {
            self->cleanup();
        }
    }
}
#endif

}
