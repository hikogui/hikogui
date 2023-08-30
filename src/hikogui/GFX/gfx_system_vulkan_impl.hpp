
#pragma once

namespace hi { inline namespace v1 {

inline gfx_system::~gfx_system()
{
    hilet lock = std::scoped_lock(gfx_system_mutex);
#ifndef NDEBUG
    intrinsic.destroy(debugUtilsMessager, nullptr, loader());
#endif
}

[[nodiscard]] inline gfx_system& gfx_system::global()
{
    if (not _global) {
        _global = std::make_unique<gfx_system>();
    }
    return *_global;
}

}} // namespace hi::v1
