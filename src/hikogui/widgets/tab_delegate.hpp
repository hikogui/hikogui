// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/tab_delegate.hpp Defines delegate_delegate and some default tab delegates.
 * @ingroup widget_delegates
 */

#pragma once

#include <memory>
#include <functional>

namespace hi { inline namespace v1 {
class tab_widget;

/** A delegate that controls the state of a tab_widget.
 *
 * @ingroup widget_delegates
 */
class tab_delegate {
public:
    using notifier_type = notifier<>;
    using callback_token = notifier_type::callback_token;
    using callback_proto = notifier_type::callback_proto;

    virtual ~tab_delegate() = default;
    virtual void init(tab_widget& sender) noexcept {}
    virtual void deinit(tab_widget& sender) noexcept {}

    virtual void add_tab(tab_widget& sender, std::size_t key, std::size_t index) noexcept {}

    virtual ssize_t index(tab_widget& sender) noexcept
    {
        return -1;
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

/** A delegate that control the state of a tab_widget.
 *
 * @ingroup widget_delegates
 * @tparam T the type used as the key for which tab is selected.
 */
template<typename T>
class default_tab_delegate : public tab_delegate {
public:
    using value_type = T;

    observer<value_type> value;
    std::unordered_map<std::size_t, std::size_t> tab_indices;

    /** Construct a default tab delegate.
     *
     * @param value The observer value which represents the selected tab.
     */
    default_tab_delegate(forward_of<observer<value_type>> auto&& value) noexcept : value(hi_forward(value))
    {
        _value_cbt = this->value.subscribe([&](auto...) {
            this->_notifier();
        });
    }

    // XXX key should really be of value_type, not sure how to handle that with the tab_widget not knowing the type of key.
    void add_tab(tab_widget& sender, std::size_t key, std::size_t index) noexcept override
    {
        hi_assert(not tab_indices.contains(key));
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

/** Create a shared pointer to a default tab delegate.
 *
 * @ingroup widget_delegates
 * @see default_tab_delegate
 * @param value The observer value which represents the selected tab.
 * @return shared pointer to a tab delegate
 */
std::shared_ptr<tab_delegate> make_default_tab_delegate(auto&& value) noexcept requires requires
{
    default_tab_delegate<observer_decay_t<decltype(value)>>{hi_forward(value)};
}
{
    using value_type = observer_decay_t<decltype(value)>;
    return std::make_shared<default_tab_delegate<value_type>>(hi_forward(value));
}

}} // namespace hi::v1
