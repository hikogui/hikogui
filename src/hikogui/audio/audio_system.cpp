// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "audio_system.hpp"
#include "audio_system_aggregate.hpp"
#include "../architecture.hpp"
#if HI_OPERATING_SYSTEM == HI_OS_WINDOWS
#include "audio_system_win32.hpp"
#endif

namespace hi::inline v1 {

audio_system::audio_system(std::weak_ptr<audio_system_delegate> delegate) :
    _delegate(std::move(delegate))
{
}

audio_system::~audio_system() {}

void audio_system::init() noexcept
{
    if (auto delegate = _delegate.lock()) {
        delegate->init(*this);
    }
}

void audio_system::deinit() noexcept
{
    if (auto delegate = _delegate.lock()) {
        delegate->deinit(*this);
    }
}

} // namespace hi::inline v1