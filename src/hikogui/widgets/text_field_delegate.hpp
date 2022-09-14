// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <string>
#include <string_view>
#include <optional>
#include "../label.hpp"

namespace hi::inline v1 {
class text_field_widget;

class text_field_delegate {
public:
    using notifier_type = notifier<>;
    using callback_token = notifier_type::callback_token;
    using callback_proto = notifier_type::callback_proto;

    virtual ~text_field_delegate() = default;
    virtual void init(text_field_widget const& sender) noexcept {}
    virtual void deinit(text_field_widget const& sender) noexcept {}

    /** Validate the text field.
     *
     * @param sender The widget that called this function.
     * @param text The text entered by the user into the text field.
     * @return no-value when valid, or a label to display to the user when invalid.
     */
    virtual label validate(text_field_widget& sender, std::string_view text) noexcept
    {
        return {};
    }

    /** Get the text to show in the text field.
     * When the user is not editing the text the text-field will request what to show
     * using this function.
     *
     * @param sender The widget that called this function.
     * @return The text to show in the text field.
     */
    virtual std::string text(text_field_widget& sender) noexcept
    {
        return {};
    }

    /** Set the text as entered by the user.
     * When the user causes a text field to commit,
     * by pressing enter, tab, or clicking outside the field and when
     * the text was validated the widget will call this function to commit the
     * text with the delegate.
     *
     * @pre text Must have been validated as correct.
     * @param sender The widget that called this function.
     * @param text The text entered by the user.
     */
    virtual void set_text(text_field_widget& sender, std::string_view text) noexcept {}

    callback_token subscribe(forward_of<callback_proto> auto&& callback, callback_flags flags = callback_flags::synchronous) noexcept
    {
        return _notifier.subscribe(hi_forward(callback), flags);
    }

protected:
    notifier_type _notifier;
};

} // namespace hi::inline v1
