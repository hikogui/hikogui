// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/selection_delegate.hpp Defines delegate_delegate and some default selection delegates.
 * @ingroup widget_delegates
 */

#pragma once

#include "../l10n/l10n.hpp"
#include "../macros.hpp"
#include "radio_button_widget.hpp"
#include <memory>
#include <functional>
#include <vector>

namespace hi { inline namespace v1 {
class selection_widget;

/** A delegate that controls the state of a selection_widget.
 *
 * @ingroup widget_delegates
 */
class selection_delegate : public button_delegate {
public:
    virtual ~selection_delegate() = default;

    [[nodiscard]] virtual std::optional<std::vector<std::unique_ptr<widget>>> make_button_widgets(widget& sender) noexcept
    {
        return std::nullopt;
    }

    [[nodiscard]] virtual std::optional<widget_id> keyboard_focus_id(widget_intf const& sender) const noexcept
    {
        return std::nullopt;
    }

    [[nodiscard]] virtual size_t number_of_options() const noexcept
    {
        return 0;
    }

    [[nodiscard]] bool has_options() const noexcept
    {
        return number_of_options() != 0;
    }

    [[nodiscard]] virtual std::optional<label> selected(widget_intf const& sender) const noexcept
    {
        return std::nullopt;
    }
};

/** A delegate that control the state of a selection_widget.
 *
 * @ingroup widget_delegates
 * @tparam T the type used as the key for which option is selected.
 */
template<typename T>
class default_selection_delegate : public selection_delegate {
public:
    using value_type = T;
    using option_type = std::pair<value_type, label>;
    using options_type = std::vector<option_type>;

    observer<value_type> value;
    observer<options_type> options;

    /** Construct a default selection delegate.
     *
     * @param value The observer value which represents the selected option.
     * @param options An observer std::vector<std::pair<value_type,label>> of all possible options.
     */
    template<forward_of<observer<value_type>> Value, forward_of<observer<options_type>> Options>
    default_selection_delegate(Value&& value, Options&& options) noexcept :
        value(std::forward<Value>(value)), options(std::forward<Options>(options))
    {
        // clang-format off
        _value_cbt = this->value.subscribe([&](auto...){ this->_notifier(); });
        _options_cbt = this->options.subscribe([&](auto...){ _options_modified = true; this->_notifier(); });
        // clang-format on
    }

    void init(widget_intf const& sender) noexcept override
    {
        _last_init_id = sender.id;
    }

    void deinit(widget_intf const& sender) noexcept override
    {
        hilet it = std::lower_bound(_senders.begin(), _senders.end(), sender.id);
        if (it != _senders.end() and it->id == sender.id) {
            _senders.erase(it);
        }
    }

    [[nodiscard]] button_state state(widget_intf const& sender) const noexcept override
    {
        hilet it = std::lower_bound(_senders.begin(), _senders.end(), sender.id);

        if (it != _senders.end() and it->id == sender.id) {
            return *value == it->value ? button_state::on : button_state::off;

        } else {
            // button-button was not yet registered.
            return button_state::off;
        }
    }

    void activate(widget_intf const& sender) noexcept override
    {
        hilet it = std::lower_bound(_senders.begin(), _senders.end(), sender.id);

        if (it != _senders.end() and it->id == sender.id) {
            value = it->value;
        }
    }

    [[nodiscard]] size_t number_of_options() const noexcept override
    {
        return options->size();
    }

    [[nodiscard]] std::optional<label> selected(widget_intf const& sender) const noexcept override
    {
        for (auto&& option : *options) {
            if (option.first == *value) {
                return option.second;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<widget_id> keyboard_focus_id(widget_intf const& sender) const noexcept override
    {
        if (_senders.empty()) {
            return std::nullopt;
        }

        for (hilet& sender : _senders) {
            if (sender.value == *value) {
                return sender.id;
            }
        }
        return _senders.front().id;
    }

    [[nodiscard]] std::optional<std::vector<std::unique_ptr<widget>>> make_button_widgets(widget& sender) noexcept override
    {
        using button_widget = radio_menu_button_widget;
        using button_attributes = radio_menu_button_widget::attributes_type;

        if (not std::exchange(_options_modified, false)) {
            return std::nullopt;
        }

        auto r = std::vector<std::unique_ptr<widget>>{};
        for (auto& option : *options) {
            r.push_back(
                std::make_unique<button_widget>(make_not_null(sender), button_attributes{option.second}, shared_from_this()));
            last_init_was_button(option.first);
        }
        return r;
    }

private:
    struct sender_info_type {
        widget_id id = {};
        value_type value;

        [[nodiscard]] friend bool operator==(sender_info_type const& lhs, widget_id const& rhs) noexcept
        {
            return lhs.id == rhs;
        }

        [[nodiscard]] friend auto operator<=>(sender_info_type const& lhs, widget_id const& rhs) noexcept
        {
            return lhs.id <=> rhs;
        }
    };

    callback<void(value_type)> _value_cbt;
    callback<void(options_type)> _options_cbt;

    bool _options_modified = true;
    widget_id _last_init_id = 0;
    std::vector<sender_info_type> _senders;

    void last_init_was_button(value_type value) noexcept
    {
        hi_assert(_last_init_id != 0);

        hilet it = std::lower_bound(_senders.begin(), _senders.end(), _last_init_id);
        hi_assert(it == _senders.end() or it->id != _last_init_id, "button was already registered with selection-delegate.");
        _senders.emplace(it, _last_init_id, value);
        _last_init_id = 0;
    }
};

template<typename Value, typename Options>
default_selection_delegate(Value&&, Options&&) -> default_selection_delegate<observer_decay_t<Value>>;

}} // namespace hi::v1
