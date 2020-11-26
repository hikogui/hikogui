// Copyright 2019 Pokitec
// All rights reserved.

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