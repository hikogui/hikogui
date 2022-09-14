// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "tab_delegate.hpp"
#include "../observer.hpp"
#include "../cast.hpp"
#include <type_traits>
#include <memory>
#include <unordered_map>

namespace hi::inline v1 {

template<typename T>
class default_tab_delegate : public tab_delegate {
public:
    using value_type = T;

    observer<value_type> value;
    std::unordered_map<std::size_t, std::size_t> tab_indices;

    default_tab_delegate(forward_of<observer<value_type>> auto&& value) noexcept : value(hi_forward(value))
    {
        _value_cbt = this->value.subscribe([&](auto...) {
            this->_notifier();
        });
    }

    // XXX key should really be of value_type, not sure how to handle that with the tab_widget not knowing the type of key.
    void add_tab(tab_widget& sender, std::size_t key, std::size_t index) noexcept override
    {
        hi_axiom(not tab_indices.contains(key));
        tab_indices[key] = index;
    }

    [[nodiscard]] ssize_t index(tab_widget& sender) noexcept override
    {
        auto it = tab_indices.find(*value);
        if (it == tab_indices.end()) {
            return -1;
        } else {
            return static_cast<ssize_t>(it->second);
        }
    }

private:
    typename decltype(value)::callback_token _value_cbt;
};

std::shared_ptr<tab_delegate> make_default_tab_delegate(auto&& value) noexcept requires requires
{
    default_tab_delegate<observer_decay_t<decltype(value)>>{hi_forward(value)};
}
{
    using value_type = observer_decay_t<decltype(value)>;
    return std::make_shared<default_tab_delegate<value_type>>(hi_forward(value));
}

} // namespace hi::inline v1
