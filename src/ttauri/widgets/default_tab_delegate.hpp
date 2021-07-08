// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "tab_delegate.hpp"
#include "../observable.hpp"
#include "../cast.hpp"
#include <type_traits>
#include <memory>
#include <unordered_map>

namespace tt {

template<typename T>
class default_tab_delegate : public tab_delegate {
public:
    using value_type = T;

    observable<value_type> value;
    std::unordered_map<size_t,size_t> tab_indices;

    template<typename Value>
    default_tab_delegate(Value &&value) noexcept :
        value(std::forward<Value>(value))    {
    }

    callback_ptr_type subscribe(tab_widget &sender, callback_ptr_type const &callback_ptr) noexcept override
    {
        value.subscribe(callback_ptr);
        return callback_ptr;
    }

    void unsubscribe(tab_widget &sender, callback_ptr_type const &callback_ptr) noexcept override
    {
        value.unsubscribe(callback_ptr);
    }

    void add_tab(tab_widget &sender, size_t key, size_t index) noexcept override
    {
        tt_axiom(not tab_indices.contains(key));
        tab_indices[key] = index;
    }

    [[nodiscard]] ssize_t index(tab_widget &sender) noexcept override
    {
        auto i = tab_indices.find(*value);
        if (i == tab_indices.end()) {
            return -1;
        } else {
            return static_cast<ssize_t>(i->second);
        }
    }
};

template<typename Value>
default_tab_delegate(Value &&) -> default_tab_delegate<observable_argument_t<std::remove_cvref_t<Value>>>;

template<typename Value>
std::unique_ptr<tab_delegate> make_unique_default_tab_delegate(Value &&value) noexcept
{
    using value_type = observable_argument_t<std::remove_cvref_t<Value>>;
    return std::make_unique<default_tab_delegate<value_type>>(std::forward<Value>(value));
}

} // namespace tt