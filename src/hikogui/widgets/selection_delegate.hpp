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
    virtual ~selection_delegate() = default;

    virtual void init(selection_widget& sender) noexcept {}

    virtual void deinit(selection_widget& sender) noexcept {}

    /** Subscribe a callback for notifying the widget of a data change.
     */
    auto subscribe(selection_widget& sender, callback_flags flags, std::invocable<> auto&& callback) noexcept
    {
        return _notifier.subscribe(flags, hi_forward(callback));
    }

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

protected:
    notifier<> _notifier;
};

} // namespace hi::inline v1
