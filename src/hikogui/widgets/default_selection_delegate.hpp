// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "selection_delegate.hpp"
#include "../observable.hpp"
#include "../label.hpp"
#include <type_traits>
#include <memory>
#include <vector>

namespace hi::inline v1 {

template<typename T>
class default_selection_delegate : public selection_delegate {
public:
    using value_type = T;

    observable<std::vector<std::pair<value_type, label>>> options;
    observable<value_type> value;
    observable<value_type> off_value;

    default_selection_delegate(auto &&options, auto &&value, auto &&off_value) noexcept :
        options(hi_forward(options)), value(hi_forward(value)), off_value(hi_forward(off_value))
    {
        // clang-format off
        _options_cbt = this->options.subscribe([&](auto...){ this->_notifier(); });
        _value_cbt = this->value.subscribe([&](auto...){ this->_notifier(); });
        _off_value_cbt = this->off_value.subscribe([&](auto...){ this->_notifier(); });
        // clang-format on
    }

    default_selection_delegate(auto &&option_list, auto &&value) noexcept :
        default_selection_delegate(hi_forward(option_list), hi_forward(value), value_type{})
    {
    }

    void set_selected(selection_widget &sender, ptrdiff_t index) noexcept override
    {
        if (index == -1 || index >= std::ssize(*options)) {
            value = *off_value;
        } else {
            value = (*options)[index].first;
        }
    }

    std::pair<std::vector<label>, ptrdiff_t> options_and_selected(selection_widget const& sender) const noexcept override
    {
        auto labels = std::vector<label>{};
        labels.reserve(options->size());

        auto index = 0_z;
        auto selected_index = -1_z;
        for (auto &&option : *options) {
            if (*value == option.first) {
                selected_index = index;
            }
            labels.push_back(option.second);
            ++index;
        }

        return {std::move(labels), selected_index};
    }

private:
    typename decltype(options)::token_type _options_cbt;
    typename decltype(value)::token_type _value_cbt;
    typename decltype(off_value)::token_type _off_value_cbt;
};

template<typename OptionList, typename Value, typename... Args>
default_selection_delegate(OptionList &&, Value &&, Args &&...)
    -> default_selection_delegate<observable_argument_t<std::remove_cvref_t<Value>>>;

template<typename OptionList, typename Value, typename... Args>
std::unique_ptr<selection_delegate>
make_unique_default_selection_delegate(OptionList &&option_list, Value &&value, Args &&...args) noexcept
{
    using value_type = observable_argument_t<std::remove_cvref_t<Value>>;
    return std::make_unique<default_selection_delegate<value_type>>(
        std::forward<OptionList>(option_list), std::forward<Value>(value), std::forward<Args>(args)...);
}

} // namespace hi::inline v1
