// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <memory>
#include <functional>

namespace hi::inline v1 {
class tab_widget;

class tab_delegate {
public:
    virtual ~tab_delegate() = default;
    virtual void init(tab_widget &sender) noexcept {}
    virtual void deinit(tab_widget &sender) noexcept {}

    /** Subscribe a callback for notifying the widget of a data change.
     */
    auto subscribe(tab_widget& sender, callback_flags flags, std::invocable<> auto&& callback) noexcept
    {
        return _notifier.subscribe(flags, hi_forward(callback));
    }

    virtual void add_tab(tab_widget& sender, std::size_t key, std::size_t index) noexcept {}

    virtual ssize_t index(tab_widget &sender) noexcept
    {
        return -1;
    }

protected:
    notifier<> _notifier;
};

} // namespace hi::inline v1
