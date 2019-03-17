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

using namespace std;
using namespace TTauri;
using namespace TTauri::GUI;

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

    self.view.wantsLayer = YES;        // Back the view with a layer created by the makeBackingLayer method.

    auto surface = [(MoltenVKView *)self.view makeVulkanLayer:callbackData.instance->intrinsic];
    auto window = make_shared<Window>(callbackData.instance.get(), surface);

    vk::Rect2D rect;
    rect.offset.x = self.view.frame.origin.x;
    rect.offset.y = self.view.frame.origin.y;
    rect.extent.width = self.view.frame.size.width;
    rect.extent.height = self.view.frame.size.height;
    window->setWindowRectangle(rect);

    if (!callbackData.instance->add(window)) {
        auto alert = [[NSAlert alloc] init];
        [alert setMessageText:@"Could not open window."];
        [alert setInformativeText:@"Window is not supported on any Metal/Vulkan device."];
        [alert addButtonWithTitle:@"Ok"];
        [alert runModal];
    }

}



@end
