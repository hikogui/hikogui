// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <memory>
#include <functional>

namespace hi::inline v1 {
class tab_widget;

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
    callback_token subscribe(forward_of<callback_proto> auto&& callback, callback_flags flags = callback_flags::synchronous) noexcept
    {
        return _notifier.subscribe(hi_forward(callback), flags);
    }

protected:
    notifier_type _notifier;
};

} // namespace hi::inline v1
