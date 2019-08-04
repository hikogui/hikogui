

#include "Application_macos.hpp"

#import <Cocoa/Cocoa.h>

namespace TTauri {

Application_macos::Application_macos() :
{
    @autoreleasepool {
        [NSApplication sharedApplication];

        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

        // Get the main path the Resources folder of the bundle by requesting a known existing file.
        auto _resourceFile = [[NSBundle mainBundle] pathForResource:@"BackingPipeline.frag" ofType:@"spv"];
        auto resourceFile = boost::filesystem::path([_resourceFile UTF8String]);
        resourceDir = resourceFile.parent_path();

        id menubar = [NSMenu new];
        id appMenuItem = [NSMenuItem new];
        [menubar addItem:appMenuItem];
        [NSApp setMainMenu:menubar];
        id appMenu = [NSMenu new];
        id appName = [[NSProcessInfo processInfo] processName];
        id quitTitle = [@"Quit " stringByAppendingString:appName];
        id quitMenuItem = [[NSMenuItem alloc] initWithTitle:quitTitle
            action:@selector(terminate:) keyEquivalent:@"q"];
        [appMenu addItem:quitMenuItem];
        [appMenuItem setSubmenu:appMenu];

        id window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 200, 200)
            styleMask:NSTitledWindowMask backing:NSBackingStoreBuffered defer:NO];
                
        [window cascadeTopLeftFromPoint:NSMakePoint(20,20)];
        [window setTitle:appName];
        [window makeKeyAndOrderFront:nil];
        
        [NSApp activateIgnoringOtherApps:YES];
    }
}

Application_macos::~Application_macos()
{
}

void Application_macos::initialize(const std::shared_ptr<ApplicationDelegate> delegate, int argc, char **argv)
{
    this->argc = argc;
    this->argv = argv;

    Application_base::initialize(move(delegate));
}

void Application_macos::runOnMainThread(std::function<void()> function)
{

    let functionP = new std::function<void()>(std::move(function));
    required_assert(functionP);

    // XXX post a message to the main thread loop.
    //auto r = PostThreadMessageW(mainThreadID, WM_APP_CALL_FUNCTION, 0, reinterpret_cast<LPARAM>(functionP));
    required_assert(r != 0);
}

int Application_macos::loop()
{
    startingLoop();

    [NSApp run];

    return 0;
}

}

