// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "audio_system.hpp"
#include "../algorithm.hpp"
#include <algorithm>

namespace tt::inline v1 {

class audio_system_aggregate : public audio_system {
public:
    using super = audio_system;

    audio_system_aggregate(tt::event_queue const &event_queue, std::weak_ptr<audio_system_delegate> delegate);

    [[nodiscard]] std::vector<audio_device *> devices() noexcept override
    {
        auto r = std::vector<audio_device *>{};
        for (auto &child : _children) {
            auto tmp = child->devices();
            std::move(tmp.begin(), tmp.end(), std::back_inserter(r));
        }
        return r;
    }

    template<typename T, typename... Args>
    audio_system &make_audio_system(Args &&...args)
    {
        auto new_audio_system = std::make_unique<T>(_event_queue, _aggregate_delegate, std::forward<Args>(args)...);
        auto new_audio_system_ptr = new_audio_system.get();

        new_audio_system->init();
        _children.push_back(std::move(new_audio_system));

        if (auto delegate = _delegate.lock()) {
            delegate->audio_device_list_changed(*this);
        }
        return *new_audio_system_ptr;
    }

private:
    std::vector<std::unique_ptr<audio_system>> _children;

    /** The child audio systems take a weak_ptr to the aggregate_delegate.
     */
    std::shared_ptr<audio_system_delegate> _aggregate_delegate;

    friend class audio_system_aggregate_delegate;
};

} // namespace tt
