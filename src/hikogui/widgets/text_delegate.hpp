// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <memory>
#include <functional>

namespace hi::inline v1 {
class text_widget;

class text_delegate {
public:
    using notifier_type = notifier<>;
    using callback_token = notifier_type::callback_token;
    using callback_proto = notifier_type::callback_proto;

    virtual ~text_delegate() = default;

    virtual void init(text_widget& sender) noexcept {}
    virtual void deinit(text_widget& sender) noexcept {}

    /** Read text as a string of graphemes.
     */
    [[nodiscard]] virtual gstring read(text_widget& sender) noexcept = 0;

    /** Write text from a string of graphemes.
     */
    virtual void write(text_widget& sender, gstring const& text) noexcept = 0;

    /** Subscribe a callback for notifying the widget of a data change.
     */
    [[nodiscard]] callback_token
    subscribe(forward_of<callback_proto> auto&& callback, callback_flags flags = callback_flags::synchronous) noexcept
    {
        return _notifier.subscribe(hi_forward(callback), flags);
    }

protected:
    notifier_type _notifier;
};

} // namespace hi::inline v1
