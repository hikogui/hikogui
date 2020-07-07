// Copyright 2019 Pokitec
// All rights reserved.

#include "ttauri/os_detect.hpp"
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#include <sys/types.h>
#include <sys/sysctl.h>

namespace tt {

void _debugger_log(char const *text) noexcept
{
    @autoreleasepool {
        NSString *tmp = [NSString stringWithCString:text encoding:NSUTF8StringEncoding];
        NSLog(@"%@", tmp);
    }
}

bool debugger_is_present() noexcept
{
    struct kinfo_proc info;
    info.kp_proc.p_flag = 0;

    int mib[4];
    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PID;
    mib[3] = getpid();

    size_t size = sizeof(info);
    sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, NULL, 0);

    return ((info.kp_proc.p_flag & P_TRACED) != 0);
}

void _debugger_dialogue(char const *caption, char const *message)
{
    @autoreleasepool {
        NSString *caption_s = [NSString stringWithCString:caption encoding:NSUTF8StringEncoding];
        NSString *message_s = [NSString stringWithCString:message encoding:NSUTF8StringEncoding];

        auto *alert = [[NSAlert alloc] init];
        [alert setMessageText:caption_s];
        [alert setInformativeText:message_s];
        [alert addButtonWithTitle:@"OK"];
        [alert setAlertStyle:NSAlertStyleCritical];
        [alert runModal];
    }
}

}
