// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/GUISystem_vulkan.hpp"

namespace TTauri {

class GUISystem_vulkan_macos final : public GUISystem_vulkan {
public:
    uint64_t hostFrequency;
    CVDisplayLinkRef updateAndRenderThread;

    bool stopUpdateAndRender = false;

    GUISystem_vulkan_macos();
    virtual ~GUISystem_vulkan_macos();

    virtual void createWindow(std::shared_ptr<Window::Delegate> windowDelegate, const std::string &title);

    static CVReturn updateAndRenderLoop(CVDisplayLinkRef displayLink, const CVTimeStamp* now, const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* target);
};

}
