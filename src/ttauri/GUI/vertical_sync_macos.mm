

#include "vertical_sync_macos.hpp"

namespace tt::inline v1 {

static CVReturn proc(CVDisplayLinkRef displayLink, const CVTimeStamp* now, const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* target)
{
    auto *self = reinterpret_cast<vertical_sync_macos*>(target);

    auto currentHostTime = static_cast<ubig128>(now->hostTime);
    currentHostTime *= 1000000000;
    currentHostTime /= self->hostFrequency;

    auto outputHostTime = static_cast<ubig128>(outputTime->hostTime);
    outputHostTime *= 1000000000;
    outputHostTime /= self->hostFrequency;

    self->callback();
}

vertical_sync_macos::vertical_sync_macos(std::function<void(void *,utc_nanoseconds)> callback, void *callbackData) noexcept :
    vertical_sync_base(std::move(callback), callbackData)
{

    hostFrequency = narrow_cast<uint64_t>(CVGetHostClockFrequency());

    // Start update loop.
    CVDisplayLinkCreateWithActi32x4GDisplays(&displayLink);
    CVDisplayLinkSetOutputCallback(displayLink, &vertical_sync_macos::proc, static_cast<void *>(this));
    CVDisplayLinkStart(displayLink);
}


vertical_sync_macos::~vertical_sync_macos()
{

}

}
