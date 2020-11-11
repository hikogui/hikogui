

#include "VerticalSync_macos.hpp"

namespace tt {

static CVReturn proc(CVDisplayLinkRef displayLink, const CVTimeStamp* now, const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* target)
{
    auto *self = reinterpret_cast<VerticalSync_macos*>(target);

    auto currentHostTime = static_cast<ubig128>(now->hostTime);
    currentHostTime *= 1000000000;
    currentHostTime /= self->hostFrequency;

    auto outputHostTime = static_cast<ubig128>(outputTime->hostTime);
    outputHostTime *= 1000000000;
    outputHostTime /= self->hostFrequency;

    self->callback();
}

VerticalSync_macos::VerticalSync_macos(std::function<void(void *,hires_utc_clock::time_point)> callback, void *callbackData) noexcept :
    VerticalSync_base(std::move(callback), callbackData)
{

    hostFrequency = narrow_cast<uint64_t>(CVGetHostClockFrequency());

    // Start update loop.
    CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
    CVDisplayLinkSetOutputCallback(displayLink, &VerticalSync_macos::proc, static_cast<void *>(this));
    CVDisplayLinkStart(displayLink);
}


VerticalSync_macos::~VerticalSync_macos()
{

}

}
