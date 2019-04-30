// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Instance_vulkan.hpp"

#import <Cocoa/Cocoa.h>

namespace TTauri {
namespace GUI {

class Instance_vulkan_macos : public Instance_vulkan {
public:
    uint64_t hostFrequency;
    CVDisplayLinkRef updateAndRenderThread;

    bool stopUpdateAndRender = false;

    Instance_vulkan_macos();
    virtual ~Instance_vulkan_macos();

    virtual void createWindow(std::shared_ptr<GUI::Window::Delegate> windowDelegate, const std::string &title);

    static CVReturn updateAndRenderLoop(CVDisplayLinkRef displayLink, const CVTimeStamp* now, const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* target);
};

}}
