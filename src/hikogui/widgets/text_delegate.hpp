// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "button_state.hpp"
#include <memory>
#include <functional>

namespace hi::inline v1 {
class text_widget;

class text_delegate {
public:
    virtual ~text_delegate() = default;

    virtual void init(text_widget &sender) noexcept {}

    virtual void deinit(text_widget &sender) noexcept {}

    /** Subscribe a callback for notifying the widget of a data change.
     */
    [[nodiscard]] auto subscribe(text_widget &sender, callback_flags flags, std::invocable<> auto &&callback) noexcept
    {
        return _notifier.subscribe(flags, hi_forward(callback));
    }

        /** Subscribe a callback for notifying the widget of a data change.
     */
    [[nodiscard]] auto subscribe(text_widget& sender, std::invocable<> auto&& callback) noexcept
    {
        return subscribe(sender, callback_flags::synchronous, hi_forward(callback));
    }

    /** Read text as a string of graphemes.
     */
    virtual gstring read(text_widget &sender) noexcept = 0;

    /** Write text from a string of graphemes.
     */
    virtual void write(text_widget &sender, gstring const &text) noexcept = 0;


protected:
    notifier<> _notifier;
};

} // namespace hi::inline v1

