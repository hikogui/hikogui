// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../label.hpp"
#include <memory>
#include <functional>
#include <vector>

namespace tt {
class selection_widget;

class selection_delegate {
public:
    using callback_ptr_type = std::shared_ptr<std::function<void()>>;

    virtual void init(selection_widget &sender) noexcept {}

    virtual void deinit(selection_widget &sender) noexcept {}

    /** Subscribe a callback for notifying the widget of a data change.
     */
    virtual callback_ptr_type subscribe(selection_widget &sender, callback_ptr_type const &callback) noexcept
    {
        return callback;
    }

    /** Subscribe a callback for notifying the widget of a data change.
     */
    template<typename Callback>
    requires(std::is_invocable_v<Callback>) [[nodiscard]] callback_ptr_type
        subscribe(selection_widget &sender, Callback &&callback) noexcept
    {
        return subscribe(sender, std::make_shared<std::function<void()>>(std::forward<Callback>(callback)));
    }

    /** Unsubscribe a callback.
     */
    virtual void unsubscribe(selection_widget &sender, callback_ptr_type const &callback) noexcept {}

    /** Called when an option is selected by the user.
     * @param index The index of the option selected, -1 if no option is selected.
     */
    virtual void set_selected(selection_widget &sender, ssize_t index) noexcept {};

    /** Retrieve the label of an option.
     */
    virtual std::pair<std::vector<label>, ssize_t> options_and_selected(selection_widget const &sender) const noexcept
    {
        return {{}, -1};
    }
};

} // namespace tt
