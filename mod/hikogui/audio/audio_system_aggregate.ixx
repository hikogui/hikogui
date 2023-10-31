// Copyright Take Vos 2020, 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <algorithm>
#include <coroutine>

export module hikogui_audio_audio_system_aggregate;
import hikogui_algorithm;
import hikogui_audio_audio_system;

export namespace hi { inline namespace v1 {

export class audio_system_aggregate : public audio_system {
public:
    using super = audio_system;

    audio_system_aggregate() = default;
    virtual ~audio_system_aggregate() {}

    [[nodiscard]] generator<audio_device &> devices() noexcept override
    {
        for (auto &child : _children) {
            for (auto &device: child.system->devices()) {
                co_yield device;
            }
        }
    }

    void add_child(std::unique_ptr<audio_system> new_child)
    {
        auto new_cbt = new_child->subscribe([this] {
            _notifier();
        });

        _children.emplace_back(std::move(new_child), std::move(new_cbt));
    }

private:
    struct child_type {
        std::unique_ptr<audio_system> system;
        callback<void()> cbt;
    };

    std::vector<child_type> _children;
};

}} // namespace hi::inline v1
