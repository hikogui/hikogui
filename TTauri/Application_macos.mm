

#include "Application_macos.hpp"

#import <Cocoa/Cocoa.h>

Application_macos::Application_macos(std::shared_ptr<Delegate> delegate, int argc, const char *argv[]) :
    Application(delegate),
    argc(argc),
    argv(argv)
{
    @autoreleasepool {
        [NSApplication sharedApplication];

        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

        // Get the main path the Resources folder of the bundle by requesting a known existing file.
        auto _resourceFile = [[NSBundle mainBundle] pathForResource:@"BackingPipeline.frag" ofType:@"spv"];
        auto resourceFile = boost::filesystem::path([_resourceFile UTF8String]);
        resourceDir = resourceFile.parent_path();

        id menubar = [[NSMenu new] autorelease];
        id appMenuItem = [[NSMenuItem new] autorelease];
        [menubar addItem:appMenuItem];
        [NSApp setMainMenu:menubar];
        id appMenu = [[NSMenu new] autorelease];
        id appName = [[NSProcessInfo processInfo] processName];
        id quitTitle = [@"Quit " stringByAppendingString:appName];
        id quitMenuItem = [[[NSMenuItem alloc] initWithTitle:quitTitle
            action:@selector(terminate:) keyEquivalent:@"q"] autorelease];
        [appMenu addItem:quitMenuItem];
        [appMenuItem setSubmenu:appMenu];

        id window = [[[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 200, 200)
            styleMask:NSTitledWindowMask backing:NSBackingStoreBuffered defer:NO]
                autorelease];
                
        [window cascadeTopLeftFromPoint:NSMakePoint(20,20)];
        [window setTitle:appName];
        [window makeKeyAndOrderFront:nil];
        
        [NSApp activateIgnoringOtherApps:YES];
    }
    return 0;
}

Application_macos::~Application_macos()
{
}

int Application_macos::loop()
{
    startingLoop();

    [NSApp run];

    return 0;
}
