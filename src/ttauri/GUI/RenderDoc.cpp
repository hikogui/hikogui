// Copyright 2020 Pokitec
// All rights reserved.

#include "RenderDoc.hpp"
#include "../logger.hpp"
#include "../URL.hpp"
#include <renderdoc/renderdoc_app.h>
#if  TT_OPERATING_SYSTEM == TT_OS_WINDOWS
#include <Windows.h>
#endif
#include <type_traits>

namespace tt {

RenderDoc::RenderDoc() noexcept {
#if TT_BUILD_TYPE == TT_BT_DEBUG
#if TT_OPERATING_SYSTEM == TT_OS_WINDOWS
    ttlet dll_urls = std::vector{
        URL{"file:renderdoc.dll"},
        URL{"file:///C:/Program%20Files/RenderDoc/renderdoc.dll"},
        URL{"file:///C:/Program%20Files%20(x86)/RenderDoc/renderdoc.dll"}
    };

    HMODULE mod = nullptr;
    for (ttlet &dll_url: dll_urls) {
        LOG_DEBUG("Trying to load renderdoc.dll at: {}", dll_url.nativePath());
        
        if ((mod = LoadLibraryW(dll_url.nativeWPath().c_str()))) {
            goto found_dll;
        }
    }
    LOG_WARNING("Could not load renderdoc.dll");
    return;

found_dll:
    pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");

    if (RENDERDOC_GetAPI == nullptr) {
        LOG_ERROR("Could not find RENDERDOC_GetAPI in renderdoc.dll");
        return;
    }

    int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_4_1, &api);
    if (ret != 1) {
        LOG_ERROR("RENDERDOC_GetAPI returns invalid value {}", ret);
        api = nullptr;
    }

    // At init, on linux/android.
    // For android replace librenderdoc.so with libVkLayer_GLES_RenderDoc.so
    //if(void *mod = dlopen("librenderdoc.so", RTLD_NOW | RTLD_NOLOAD))
    //{
    //    pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)dlsym(mod, "RENDERDOC_GetAPI");
    //    int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void **)&rdoc_api);
    //    assert(ret == 1);
    //}

    set_overlay(false, false, false);
#endif
#endif
}

void RenderDoc::set_overlay(bool frameRate, bool frameNumber, bool captureList) noexcept {
    if (!api) {
        return;
    }

    uint32_t or_mask = eRENDERDOC_Overlay_None;
    uint32_t and_mask = eRENDERDOC_Overlay_None;

    if (frameRate || frameNumber || captureList) {
        or_mask |= eRENDERDOC_Overlay_Enabled;
    } else {
        and_mask |= eRENDERDOC_Overlay_Enabled;
    }

    if (frameRate) {
        or_mask |= eRENDERDOC_Overlay_FrameRate;
    } else {
        and_mask |= eRENDERDOC_Overlay_FrameRate;
    }

    if (frameNumber) {
        or_mask |= eRENDERDOC_Overlay_FrameNumber;
    } else {
        and_mask |= eRENDERDOC_Overlay_FrameNumber;
    }

    if (captureList) {
        or_mask |= eRENDERDOC_Overlay_CaptureList;
    } else {
        and_mask |= eRENDERDOC_Overlay_CaptureList;
    }

    auto *api_ = reinterpret_cast<RENDERDOC_API_1_4_1 *>(api);

    and_mask = ~and_mask;
    api_->MaskOverlayBits(and_mask, or_mask);
}

}
