// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "audio_system.hpp"
#include "audio_system_aggregate.hpp"
#include "../architecture.hpp"
#if TT_OPERATING_SYSTEM == TT_OS_WINDOWS
#include "audio_system_win32.hpp"
#endif

namespace tt {

[[nodiscard]] audio_system *audio_system::subsystem_init() noexcept
{
    auto tmp = new audio_system_aggregate(std::make_unique<audio_system_delegate>());
    tmp->init();
    if constexpr (operating_system::current == operating_system::windows) {
        tmp->make_audio_system<audio_system_win32>();
    }
    return tmp;
}

void audio_system::subsystem_deinit() noexcept
{
    if (auto tmp = _global.exchange(nullptr)) {
        tmp->deinit();
        delete tmp;
    }
}

audio_system::audio_system(unique_or_borrow_ptr<audio_system_delegate> delegate) : _delegate(std::move(delegate)) {}

audio_system::~audio_system() {}

void audio_system::init() noexcept
{
    _delegate->init(*this);
}

void audio_system::deinit() noexcept
{
    _delegate->deinit(*this);
}

} // namespace tt