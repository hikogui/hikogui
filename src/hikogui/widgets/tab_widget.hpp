// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/tab_widget.hpp Defines tab_widget.
 * @ingroup widgets
 */

#pragma once

#include "widget.hpp"
#include "grid_widget.hpp"
#include "tab_delegate.hpp"

namespace hi { inline namespace v1 {

/** A graphical element that shows only one of a predefined set of mutually
 * exclusive child widgets.
 *
 * A tab widget is generally controlled by a `toolbar_tab_button_widget` or
 * another selection widget.
 *
 * @image html toolbar_tab_button_widget.gif
 *
 * In the following example we create three tabs on the window which observes a
 * `value` controlled by a set of toolbar tab buttons. Each tab is configured
 * with a different value: 0, 1 and 2.
 *
 * @snippet widgets/tab_example.cpp Create three tabs
 *
 * @ingroup widgets
 * @note A `tab_button` is not directly controlled by a
 *       `toolbar_tab_button_widget`. This is accomplished by sharing a delegate
 *       or a observer between the toolbar tab button and the tab widget.
 */
class tab_widget final : public widget {
public:
    using super = widget;
    using delegate_type = tab_delegate;

    std::shared_ptr<delegate_type> delegate;

    ~tab_widget();

    /** Construct a tab widget with a delegate.
     *
     * @param parent The owner of this widget.
     * @param delegate The delegate that will control this widget.
     */
    tab_widget(widget *parent, std::shared_ptr<delegate_type> delegate) noexcept;

    /** Construct a tab widget with an observer value.
     *
     * @param parent The owner of this widget.
     * @param value The value or observer value to monitor for which child widget
     *              to display.
     */
    tab_widget(widget *parent, different_from<std::shared_ptr<delegate_type>> auto&& value) noexcept requires
        requires
    {
        make_default_tab_delegate(hi_forward(value));
    } : tab_widget(parent, make_default_tab_delegate(hi_forward(value))) {}

    /** Make and add a child widget.
     *
     * @pre A widget with the same @a value must not have been added before.
     * @tparam WidgetType The type of the widget to create.
     * @tparam Key The type of the key, must be convertible to `std::size_t`.
     * @param key The value used as a key to select this newly added widget.
     * @param args The arguments to pass to the constructor of widget to add.
     */
    template<typename WidgetType, typename Key, typename... Args>
    WidgetType& make_widget(Key const& key, Args&&...args)
    {
        hi_axiom(loop::main().on_thread());

        auto tmp = std::make_unique<WidgetType>(this, std::forward<Args>(args)...);
        auto& ref = *tmp;

        hi_assert_not_null(delegate);
        delegate->add_tab(*this, static_cast<std::size_t>(key), size(_children));
        _children.push_back(std::move(tmp));
        ++global_counter<"tab_widget:make_widget:constrain">;
        process_event({gui_event_type::window_reconstrain});
        return ref;
    }

    /// @privatesection
    [[nodiscard]] generator<widget *> children() const noexcept override
    {
        for (hilet& child : _children) {
            co_yield child.get();
        }
    }

    widget_constraints const& set_constraints(set_constraints_context const& context) noexcept override;
    void set_layout(widget_layout const& context) noexcept override;
    void draw(draw_context const& context) noexcept override;
    [[nodiscard]] hitbox hitbox_test(point3 position) const noexcept override;
    [[nodiscard]] widget const *find_next_widget(
        widget const *current_widget,
        keyboard_focus_group group,
        keyboard_focus_direction direction) const noexcept override;
    /// @endprivatsectopn
private:
    widget const *_previous_selected_child = nullptr;
    std::vector<std::unique_ptr<widget>> _children;
    notifier<>::callback_token _delegate_cbt;

    using const_iterator = decltype(_children)::const_iterator;

    [[nodiscard]] const_iterator find_selected_child() const noexcept;
    [[nodiscard]] widget& selected_child() const noexcept;
};

}} // namespace hi::v1
