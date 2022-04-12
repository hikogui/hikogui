// Copyright 2019 Pokitec
// All rights reserved.

#include "architecture.hpp"
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#include <sys/types.h>
#include <sys/sysctl.h>

namespace hi::inline v1 {

bool debugger_is_present() noexcept
{
    struct kinfo_proc info;
    info.kp_proc.p_flag = 0;

    int mib[4];
    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PID;
    mib[3] = getpid();

    std::size_t size = sizeof(info);
    sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, NULL, 0);

    return ((info.kp_proc.p_flag & P_TRACED) != 0);
}


}
