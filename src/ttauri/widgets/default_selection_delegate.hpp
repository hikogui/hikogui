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

namespace tt {

template<typename T>
class default_selection_delegate : public selection_delegate {
public:
    using value_type = T;

    observable<std::vector<std::pair<value_type, label>>> options;
    observable<value_type> value;
    observable<value_type> off_value;

    template<typename OptionList, typename Value, typename OffValue>
    default_selection_delegate(OptionList &&option_list, Value &&value, OffValue &&off_value) noexcept :
        options(std::forward<OptionList>(option_list)),
        value(std::forward<Value>(value)),
        off_value(std::forward<OffValue>(off_value))
    {
    }

    template<typename OptionList, typename Value>
    default_selection_delegate(OptionList &&option_list, Value &&value) noexcept :
        options(std::forward<OptionList>(option_list)), value(std::forward<Value>(value)), off_value(value_type{})
    {
    }

    callback_ptr_type subscribe(selection_widget &sender, callback_ptr_type const &callback_ptr) noexcept override
    {
        value.subscribe(callback_ptr);
        options.subscribe(callback_ptr);
        return callback_ptr;
    }

    void unsubscribe(selection_widget &sender, callback_ptr_type const &callback_ptr) noexcept override
    {
        value.unsubscribe(callback_ptr);
        options.unsubscribe(callback_ptr);
    }

    void set_selected(selection_widget &sender, ssize_t index) noexcept override
    {
        auto options_ = *options;
        if (index == -1 || index >= std::ssize(options_)) {
            value = *off_value;
        } else {
            value = options_[index].first;
        }
    }

    std::pair<std::vector<label>, ssize_t> options_and_selected(selection_widget const &sender) const noexcept override
    {
        ttlet value_ = *value;
        auto options_ = *options;

        auto labels = std::vector<label>{};
        labels.reserve(std::size(options_));

        auto index = 0_z;
        auto selected_index = -1_z;
        for (auto &&option : options_) {
            if (value_ == option.first) {
                selected_index = index;
            }
            labels.push_back(std::move(option.second));
            ++index;
        }

        return {std::move(labels), selected_index};
    }
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

} // namespace tt