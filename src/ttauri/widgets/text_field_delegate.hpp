// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <string>
#include <string_view>
#include <optional>
#include "../label.hpp"

namespace tt::inline v1 {
class text_field_widget;

class text_field_delegate {
public:
    using callback_ptr_type = std::shared_ptr<std::function<void()>>;

    virtual ~text_field_delegate() = default;
    virtual void init(text_field_widget const &sender) noexcept {}
    virtual void deinit(text_field_widget const &sender) noexcept {}

    virtual callback_ptr_type subscribe(text_field_widget &sender, callback_ptr_type const &callback_ptr) noexcept
    {
        return callback_ptr;
    }

    /** Subscribe a callback for notifying the widget of a data change.
     */
    template<typename Callback>
    requires(std::is_invocable_v<Callback>) [[nodiscard]] callback_ptr_type
        subscribe(text_field_widget &sender, Callback &&callback) noexcept
    {
        return subscribe(sender, std::make_shared<std::function<void()>>(std::forward<Callback>(callback)));
    }

    /** Validate the text field.
     * @param text The text entered by the user into the text field.
     * @return no-value when valid, or a label to display to the user when invalid.
     */
    virtual label validate(text_field_widget &sender, std::string_view text) noexcept
    {
        return {};
    }

    /** Get the text to show in the text field.
     * When the user is not editing the text the text-field will request what to show
     * using this function.
     *
     * @return The text to show in the text field.
     */
    virtual std::string text(text_field_widget &sender) noexcept
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
     * @param text The text entered by the user.
     */
    virtual void set_text(text_field_widget &sender, std::string_view text) noexcept {}
};

} // namespace tt::inline v1
