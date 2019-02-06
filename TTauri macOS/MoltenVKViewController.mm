//
//  MoltenVKViewController.m
//  TTauri macOS
//
//  Created by Tjienta Vara on 2019-01-31.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#import "MoltenVKViewController.h"
#import "MoltenVKView.h"
#include "GUI.hpp"

using namespace std;
using namespace TTauri::Toolkit::GUI;

@implementation MoltenVKViewController
{
    CVDisplayLinkRef                _displayLink;
    shared_ptr<Instance>            instance;
}

-(void) dealloc {
    CVDisplayLinkStop(_displayLink);
    CVDisplayLinkRelease(_displayLink);
    instance = nullptr;
}

/** Since this is a single-view app, initialize Vulkan during view loading. */
-(void) viewDidLoad {
    [super viewDidLoad];

    auto extensions = vector<const char *>{
        VK_MVK_MACOS_SURFACE_EXTENSION_NAME
    };
    instance = make_shared<Instance>(extensions);
    instance->setPreferedDeviceUUID({});

    self.view.wantsLayer = YES;        // Back the view with a layer created by the makeBackingLayer method.

    auto surface = [(MoltenVKView *)self.view makeVulkanLayer:instance->intrinsic];
    auto window = make_shared<Window>(instance.get(), surface);
    instance->add(window);

    // Creates  a high performance frame refresh thread with CoreVideo, synchronized with the vertical retrace of
    // all active displays.
    CVDisplayLinkCreateWithActiveCGDisplays(&_displayLink);
    CVDisplayLinkSetOutputCallback(_displayLink, &DisplayLinkCallback, NULL);
    CVDisplayLinkStart(_displayLink);
}


#pragma mark Display loop callback function

/** Rendering loop callback function for use with a CVDisplayLink. */
static CVReturn DisplayLinkCallback(CVDisplayLinkRef displayLink,
                                    const CVTimeStamp* now,
                                    const CVTimeStamp* outputTime,
                                    CVOptionFlags flagsIn,
                                    CVOptionFlags* flagsOut,
                                    void* target) {
    //demo_draw((struct demo*)target);
    return kCVReturnSuccess;
}

@end
