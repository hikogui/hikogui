//
//  MoltenVKViewController.m
//  TTauri macOS
//
//  Created by Tjienta Vara on 2019-01-31.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#include <boost/filesystem.hpp>
#import "MoltenVKViewController.h"
#import "MoltenVKView.h"
#include "TTauri/Application.hpp"
#include "TTauri/ImageView.hpp"

using namespace std;
using namespace TTauri;

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

    // Get the main path the Resources folder of the bundle by requesting a known existing file.
    auto _resourceFile = [[NSBundle mainBundle] pathForResource:@"BackingPipeline.frag" ofType:@"spv"];
    auto resourceFile = boost::filesystem::path([_resourceFile UTF8String]);
    auto resourceDir = resourceFile.parent_path();

    app = make_shared<Application>(resourceDir);

    auto extensions = vector<const char *>{
        VK_MVK_MACOS_SURFACE_EXTENSION_NAME
    };
    callbackData.instance = make_shared<Instance>(extensions);
    callbackData.instance->setPreferedDeviceUUID({});

    self.view.wantsLayer = YES;        // Back the view with a layer created by the makeBackingLayer method.

    auto surface = [(MoltenVKView *)self.view makeVulkanLayer:callbackData.instance->intrinsic];
    auto window = make_shared<Window>(callbackData.instance.get(), surface);

    vk::Rect2D rect;
    rect.offset.x = self.view.frame.origin.x;
    rect.offset.y = self.view.frame.origin.y;
    rect.extent.width = self.view.frame.size.width;
    rect.extent.height = self.view.frame.size.height;
    window->setWindowRectangle(rect);

    auto view1 = std::make_shared<ImageView>(app->resourceDir / "lena.png");
    view1->setRectangle({ 100.0, 100.0, 1.0 }, { 200.0, 100.0, 0.0 });
    auto view2 = std::make_shared<ImageView>(app->resourceDir / "lena.png");
    view2->setRectangle({ 200.0, 200.0, 1.0 }, { 200.0, 100.0, 0.0 });
    window->view->add(view1);
    window->view->add(view2);

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

    @autoreleasepool {
        callbackData->instance->updateAndRender(boost::numeric_cast<uint64_t>(currentHostTime), boost::numeric_cast<uint64_t>(outputHostTime), false);
    }

    //demo_draw((struct demo*)target);
    return kCVReturnSuccess;
}

@end
