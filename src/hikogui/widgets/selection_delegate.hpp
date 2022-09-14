// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../label.hpp"
#include <memory>
#include <functional>
#include <vector>

namespace hi::inline v1 {
class selection_widget;

class selection_delegate {
public:
    using notifier_type = notifier<>;
    using callback_token = notifier_type::callback_token;
    using callback_proto = notifier_type::callback_proto;

    virtual ~selection_delegate() = default;

    virtual void init(selection_widget& sender) noexcept {}

    virtual void deinit(selection_widget& sender) noexcept {}

    /** Called when an option is selected by the user.
     *
     * @param sender The widget that called this function.
     * @param index The index of the option selected, -1 if no option is selected.
     */
    virtual void set_selected(selection_widget& sender, ssize_t index) noexcept {};

    /** Retrieve the label of an option.
     *
     * @param sender The widget that called this function.
     */
    virtual std::pair<std::vector<label>, ssize_t> options_and_selected(selection_widget const& sender) const noexcept
    {
        return {{}, -1};
    }

    /** Subscribe a callback for notifying the widget of a data change.
     */
    callback_token
    subscribe(forward_of<callback_proto> auto&& callback, callback_flags flags = callback_flags::synchronous) noexcept
    {
        return _notifier.subscribe(hi_forward(callback), flags);
    }

protected:
    notifier_type _notifier;
};

template<typename T>
class default_selection_delegate : public selection_delegate {
public:
    using value_type = T;
    using option_type = std::pair<value_type, label>;
    using options_type = std::vector<option_type>;

    observer<value_type> value;
    observer<options_type> options;
    observer<value_type> off_value;

    default_selection_delegate(
        forward_of<observer<value_type>> auto&& value,
        forward_of<observer<options_type>> auto&& options,
        forward_of<observer<value_type>> auto&& off_value) noexcept :
        value(hi_forward(value)), options(hi_forward(options)), off_value(hi_forward(off_value))
    {
        // clang-format off
        _value_cbt = this->value.subscribe([&](auto...){ this->_notifier(); });
        _options_cbt = this->options.subscribe([&](auto...){ this->_notifier(); });
        _off_value_cbt = this->off_value.subscribe([&](auto...){ this->_notifier(); });
        // clang-format on
    }

    default_selection_delegate(
        forward_of<observer<value_type>> auto&& value,
        forward_of<observer<options_type>> auto&& option_list) noexcept :
        default_selection_delegate(hi_forward(value), hi_forward(option_list), value_type{})
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
    typename decltype(value)::callback_token _value_cbt;
    typename decltype(options)::callback_token _options_cbt;
    typename decltype(off_value)::callback_token _off_value_cbt;
};

template<typename Value, typename OptionList, typename... Args>
[[nodiscard]] std::shared_ptr<selection_delegate>
make_default_selection_delegate(Value&& value, OptionList&& option_list, Args&&...args) noexcept requires requires
{
    default_selection_delegate<observer_decay_t<Value>>{
        std::forward<Value>(value), std::forward<OptionList>(option_list), std::forward<Args>(args)...};
}
{
    return std::make_shared<default_selection_delegate<observer_decay_t<Value>>>(
        std::forward<Value>(value), std::forward<OptionList>(option_list), std::forward<Args>(args)...);
}

} // namespace hi::inline v1
