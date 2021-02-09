// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "audio_system.hpp"

namespace tt {

audio_system::audio_system(std::weak_ptr<audio_system_delegate> const &delegate) :
    _delegate(delegate)
{
}

audio_system::~audio_system()
{
}

void audio_system::init() noexcept
{

}

}