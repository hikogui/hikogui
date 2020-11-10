// Copyright 2020 Pokitec
// All rights reserved.

#include "audio_system.hpp"
#include "../algorithm.hpp"
#include <algorithm>

namespace tt {

class audio_system_aggregate : public audio_system, public audio_system_delegate {
public:
    using super = audio_system;

    audio_system_aggregate(audio_system_delegate *delegate) : super(delegate) {}

    [[nodiscard]] std::vector<std::shared_ptr<audio_device>> devices() noexcept override
    {
        ttlet lock = std::scoped_lock(audio_system::mutex);

        auto r = std::vector<std::shared_ptr<audio_device>>{};
        for (auto &child: _children) {
            auto tmp = child->devices();
            std::move(tmp.begin(), tmp.end(), std::back_inserter(r));
        }
        return r;
    }

    void add_audio_system(std::shared_ptr<audio_system> const &new_audio_system) noexcept
    {
        ttlet lock = std::scoped_lock(audio_system::mutex);
        _children.push_back(new_audio_system);
    }

    template<typename T, typename... Args>
    std::shared_ptr<audio_system> make_audio_system(Args &&... args)
    {
        ttlet lock = std::scoped_lock(audio_system::mutex);

        auto new_audio_system = std::make_shared<T>(this, std::forward<Args>(args)...);
        new_audio_system->initialize();
        add_audio_system(new_audio_system);
        return new_audio_system;
    }

    void audio_device_list_changed() override {
        return _delegate->audio_device_list_changed();
    }

private:
    std::vector<std::shared_ptr<audio_system>> _children;
};

}
