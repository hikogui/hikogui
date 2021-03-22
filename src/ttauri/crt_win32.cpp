
#define TT_CRT_NO_MAIN 1
#include "crt.hpp"
#include "logger.hpp"
#include <Windows.h>

namespace tt {

static void crt_configure_process_leap_seconds()
{
    PROCESS_LEAP_SECOND_INFO LeapSecondInfo;
    ZeroMemory(&LeapSecondInfo, sizeof(LeapSecondInfo));
    LeapSecondInfo.Flags = PROCESS_LEAP_SECOND_INFO_FLAG_ENABLE_SIXTY_SECOND;

    auto Success = SetProcessInformation(GetCurrentProcess(), ProcessLeapSecondInfo, &LeapSecondInfo, sizeof(LeapSecondInfo));
    if (!Success) {
        tt_log_fatal("Set Leap Second priority failed: {}\n", tt::get_last_error_message());
    }
}

void crt_configure_process() noexcept
{
    crt_configure_process_leap_seconds();

}


}
