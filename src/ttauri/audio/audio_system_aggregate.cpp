// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "audio_system_aggregate.hpp"

namespace tt::inline v1 {

class audio_system_aggregate_delegate : public audio_system_delegate {
public:
    audio_system_aggregate_delegate(audio_system_aggregate &owner) : _owner(owner) {}

    void audio_device_list_changed(audio_system &self) override
    {
        if (auto delegate = _owner._delegate.lock()) {
            delegate->audio_device_list_changed(_owner);
        }
    }

private:
    audio_system_aggregate &_owner;
};

audio_system_aggregate::audio_system_aggregate(
    tt::event_queue const &event_queue,
    std::weak_ptr<audio_system_delegate> delegate) :
    super(event_queue, std::move(delegate)), _aggregate_delegate(std::make_shared<audio_system_aggregate_delegate>(*this))
{
}

} // namespace tt::inline v1
