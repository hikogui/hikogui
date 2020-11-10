// Copyright 2019 Pokitec
// All rights reserved.

#include "audio_system.hpp"

namespace tt {

audio_system::audio_system(audio_system_delegate *delegate) :
    _delegate(delegate)
{
}

audio_system::~audio_system()
{
}

void audio_system::initialize() noexcept
{

}

}