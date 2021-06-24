// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "audio_system.hpp"
#include "../algorithm.hpp"
#include <algorithm>

namespace tt {

class audio_system_aggregate : public audio_system {
public:
    using super = audio_system;

    audio_system_aggregate(unique_or_borrow_ptr<audio_system_delegate> delegate);

    [[nodiscard]] std::vector<std::shared_ptr<audio_device>> devices() noexcept override
    {
        ttlet lock = std::scoped_lock(audio_system::mutex);

        auto r = std::vector<std::shared_ptr<audio_device>>{};
        for (auto &child : _children) {
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
    std::shared_ptr<audio_system> make_audio_system(Args &&...args)
    {
        auto new_audio_system = std::make_shared<T>(*_aggregate_delegate, std::forward<Args>(args)...);
        new_audio_system->init();
        add_audio_system(new_audio_system);
        delegate().audio_device_list_changed(*this);
        return new_audio_system;
    }

private:
    std::vector<std::shared_ptr<audio_system>> _children;
    unique_or_borrow_ptr<audio_system_delegate> _aggregate_delegate;

    friend class audio_system_aggregate_delegate;
};

} // namespace tt
