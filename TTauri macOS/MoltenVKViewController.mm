//
//  MoltenVKViewController.m
//  TTauri macOS
//
//  Created by Tjienta Vara on 2019-01-31.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#import "MoltenVKViewController.h"
#import "MoltenVKView.h"
#include "TTauri/Toolkit/GUI/GUI.hpp"

using namespace std;
using namespace TTauri::Toolkit::GUI;

struct CallbackData {
    shared_ptr<Instance> instance;
    uint64_t hostFrequency;
};

@implementation MoltenVKViewController
{
    CVDisplayLinkRef _displayLink;
    CallbackData callbackData;
}

-(void) dealloc {
    CVDisplayLinkStop(_displayLink);
    CVDisplayLinkRelease(_displayLink);
    callbackData.instance = nullptr;
}

/** Since this is a single-view app, initialize Vulkan during view loading. */
-(void) viewDidLoad {
    [super viewDidLoad];

    auto extensions = vector<const char *>{
        VK_MVK_MACOS_SURFACE_EXTENSION_NAME
    };
    callbackData.instance = make_shared<Instance>(extensions);
    callbackData.instance->setPreferedDeviceUUID({});

    self.view.wantsLayer = YES;        // Back the view with a layer created by the makeBackingLayer method.

    auto surface = [(MoltenVKView *)self.view makeVulkanLayer:callbackData.instance->intrinsic];
    auto window = make_shared<Window>(callbackData.instance.get(), surface);

    window->displayRectangle.offset.x = self.view.frame.origin.x;
    window->displayRectangle.offset.y = self.view.frame.origin.y;
    window->displayRectangle.extent.width = self.view.frame.size.width;
    window->displayRectangle.extent.height = self.view.frame.size.height;

    if (!callbackData.instance->add(window)) {
        auto alert = [[NSAlert alloc] init];
        [alert setMessageText:@"Could not open window."];
        [alert setInformativeText:@"Window is not supported on any Metal/Vulkan device."];
        [alert addButtonWithTitle:@"Ok"];
        [alert runModal];
    }

    // Creates  a high performance frame refresh thread with CoreVideo, synchronized with the vertical retrace of
    // all active displays.

    callbackData.hostFrequency = boost::numeric_cast<uint64_t>(CVGetHostClockFrequency());

    CVDisplayLinkCreateWithActiveCGDisplays(&_displayLink);
    CVDisplayLinkSetOutputCallback(_displayLink, &DisplayLinkCallback, static_cast<void *>(&callbackData));
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

    auto callbackData = static_cast<CallbackData *>(target);

    auto currentHostTime = static_cast<__uint128_t>(now->hostTime);
    currentHostTime *= 1000000000;
    currentHostTime /= callbackData->hostFrequency;

    auto outputHostTime = static_cast<__uint128_t>(outputTime->hostTime);
    outputHostTime *= 1000000000;
    outputHostTime /= callbackData->hostFrequency;

    callbackData->instance->frameUpdate(boost::numeric_cast<uint64_t>(currentHostTime), boost::numeric_cast<uint64_t>(outputHostTime));

    //demo_draw((struct demo*)target);
    return kCVReturnSuccess;
}

@end
