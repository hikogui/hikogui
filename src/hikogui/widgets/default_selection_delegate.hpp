// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "selection_delegate.hpp"
#include "../observer.hpp"
#include "../label.hpp"
#include <type_traits>
#include <memory>
#include <vector>

namespace hi::inline v1 {

template<typename T>
class default_selection_delegate : public selection_delegate {
public:
    using value_type = T;
    using option_type = std::pair<value_type, label>;
    using options_type = std::vector<option_type>;

    observer<options_type> options;
    observer<value_type> value;
    observer<value_type> off_value;

    default_selection_delegate(
        forward_of<observer<options_type>> auto&& options,
        forward_of<observer<value_type>> auto&& value,
        forward_of<observer<value_type>> auto&& off_value) noexcept :
        options(hi_forward(options)), value(hi_forward(value)), off_value(hi_forward(off_value))
    {
        // clang-format off
        _options_cbt = this->options.subscribe([&](auto...){ this->_notifier(); });
        _value_cbt = this->value.subscribe([&](auto...){ this->_notifier(); });
        _off_value_cbt = this->off_value.subscribe([&](auto...){ this->_notifier(); });
        // clang-format on
    }

    default_selection_delegate(
        forward_of<observer<options_type>> auto&& option_list,
        forward_of<observer<value_type>> auto&& value) noexcept :
        default_selection_delegate(hi_forward(option_list), hi_forward(value), value_type{})
    {
    }

    void set_selected(selection_widget& sender, ptrdiff_t index) noexcept override
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
        for (auto&& option : *options) {
            if (*value == option.first) {
                selected_index = index;
            }
            labels.push_back(option.second);
            ++index;
        }

        return {std::move(labels), selected_index};
    }

private:
    typename decltype(options)::callback_token _options_cbt;
    typename decltype(value)::callback_token _value_cbt;
    typename decltype(off_value)::callback_token _off_value_cbt;
};

template<typename OptionList, typename Value, typename... Args>
[[nodiscard]] std::shared_ptr<selection_delegate>
make_default_selection_delegate(OptionList&& option_list, Value&& value, Args&&...args) noexcept requires requires
{
    default_selection_delegate<observer_argument_t<Value>>{
        std::forward<OptionList>(option_list), std::forward<Value>(value), std::forward<Args>(args)...};
}
{
    using value_type = observer_argument_t<Value>;
    return std::make_shared<default_selection_delegate<value_type>>(
        std::forward<OptionList>(option_list), std::forward<Value>(value), std::forward<Args>(args)...);
}

} // namespace hi::inline v1
