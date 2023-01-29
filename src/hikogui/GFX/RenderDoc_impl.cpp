// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "../utility/win32_headers.hpp"

#include "RenderDoc.hpp"
#include "renderdoc_app.h"
#include "../log.hpp"
#include <type_traits>
#include <filesystem>

namespace hi::inline v1 {

RenderDoc::RenderDoc() noexcept
{
#ifndef NDEBUG
#if HI_OPERATING_SYSTEM == HI_OS_WINDOWS
    hilet dll_urls = std::vector{
        std::filesystem::path{"renderdoc.dll"},
        std::filesystem::path{"C:/Program Files/RenderDoc/renderdoc.dll"},
        std::filesystem::path{"C:/Program Files (x86)/RenderDoc/renderdoc.dll"}};

    auto mod = [&]() -> HMODULE {
        for (hilet& dll_url : dll_urls) {
            hi_log_debug("Trying to load: {}", dll_url.string());

            if (auto mod = LoadLibraryW(dll_url.native().c_str())) {
                return mod;
            }
        }
        return nullptr;
    }();

    if (mod == nullptr) {
        hi_log_warning("Could not load renderdoc.dll");
        return;
    }

    auto RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
    if (RENDERDOC_GetAPI == nullptr) {
        hi_log_error("Could not find RENDERDOC_GetAPI in renderdoc.dll");
        return;
    }

    int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_4_1, &api);
    if (ret != 1) {
        hi_log_error("RENDERDOC_GetAPI returns invalid value {}", ret);
        api = nullptr;
    }

    // At init, on linux/android.
    // For android replace librenderdoc.so with libVkLayer_GLES_RenderDoc.so
    // if(void *mod = dlopen("librenderdoc.so", RTLD_NOW | RTLD_NOLOAD))
    //{
    //    pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)dlsym(mod, "RENDERDOC_GetAPI");
    //    int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void **)&rdoc_api);
    //    assert(ret == 1);
    //}

    set_overlay(false, false, false);
#endif
#endif
}

void RenderDoc::set_overlay(bool frameRate, bool frameNumber, bool captureList) noexcept
{
    if (not api) {
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

    auto& api_ = *static_cast<RENDERDOC_API_1_4_1 *>(api);

    and_mask = ~and_mask;
    api_.MaskOverlayBits(and_mask, or_mask);
}

} // namespace hi::inline v1
