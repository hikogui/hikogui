//
//  MoltenVKViewController.m
//  TTauri macOS
//
//  Created by Tjienta Vara on 2019-01-31.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#import "MoltenVKViewController.h"
#include "cube.h"

@implementation MoltenVKViewController
{
    CVDisplayLinkRef    _displayLink;
    struct demo demo;
}

-(void) dealloc {
    demo_cleanup(&demo);
    CVDisplayLinkRelease(_displayLink);
}

/** Since this is a single-view app, initialize Vulkan during view loading. */
-(void) viewDidLoad {
    [super viewDidLoad];

    self.view.wantsLayer = YES;        // Back the view with a layer created by the makeBackingLayer method.
    const char* arg = "cube";
    demo_main(&demo, (__bridge void *)self.view, 1, &arg);

    CVDisplayLinkCreateWithActiveCGDisplays(&_displayLink);
    CVDisplayLinkSetOutputCallback(_displayLink, &DisplayLinkCallback, &demo);
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
    demo_draw((struct demo*)target);
    return kCVReturnSuccess;
}

@end
