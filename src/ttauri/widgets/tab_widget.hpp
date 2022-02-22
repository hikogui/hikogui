// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "grid_widget.hpp"
#include "tab_delegate.hpp"
#include "default_tab_delegate.hpp"

namespace tt::inline v1 {

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
 * @note A `tab_button` is not directly controlled by a
 *       `toolbar_tab_button_widget`. This is accomplished by sharing a delegate
 *       or a observable between the toolbar tab button and the tab widget.
 */
class tab_widget final : public widget {
public:
    using super = widget;
    using delegate_type = tab_delegate;

    ~tab_widget();

    /** Construct a tab widget with a delegate.
     *
     * @param window The window that this widget is shown on.
     * @param parent The owner of this widget.
     * @param delegate The delegate that will control this widget.
     */
    tab_widget(gui_window &window, widget *parent, std::weak_ptr<delegate_type> delegate) noexcept;

    /** Construct a tab widget with an observable value.
     *
     * @param window The window that this widget is shown on.
     * @param parent The owner of this widget.
     * @param value The value or observable value to monitor for which child widget
     *              to display.
     */
    template<typename Value>
    tab_widget(gui_window &window, widget *parent, Value &&value) noexcept
        requires(not std::is_convertible_v<Value, weak_or_unique_ptr<delegate_type>>) :
        tab_widget(window, parent, make_unique_default_tab_delegate(std::forward<Value>(value)))
    {
    }

    /** Make and add a child widget.
     *
     * @pre A widget with the same @a value must not have been added before.
     * @tparam WidgetType The type of the widget to create.
     * @tparam Key The type of the key, must be convertible to `std::size_t`.
     * @param key The value used as a key to select this newly added widget.
     * @param args The arguments to pass to the constructor of widget to add.
     */
    template<typename WidgetType, typename Key, typename... Args>
    WidgetType &make_widget(Key const &key, Args &&...args)
    {
        tt_axiom(is_gui_thread());

        auto tmp = std::make_unique<WidgetType>(window, this, std::forward<Args>(args)...);
        auto &ref = *tmp;
        if (auto delegate = _delegate.lock()) {
            delegate->add_tab(*this, static_cast<std::size_t>(key), size(_children));
        }
        _children.push_back(std::move(tmp));
        request_reconstrain();
        return ref;
    }

    /// @privatesection
    [[nodiscard]] pmr::generator<widget *> children(std::pmr::polymorphic_allocator<> &) const noexcept override
    {
        for (ttlet &child : _children) {
            co_yield child.get();
        }
    }

    widget_constraints const &set_constraints() noexcept override;
    void set_layout(widget_layout const &layout) noexcept override;
    void draw(draw_context const &context) noexcept override;
    [[nodiscard]] hitbox hitbox_test(point3 position) const noexcept override;
    [[nodiscard]] widget const *find_next_widget(
        widget const *current_widget,
        keyboard_focus_group group,
        keyboard_focus_direction direction) const noexcept override;
    /// @endprivatsectopn
private:
    widget const *_previous_selected_child = nullptr;
    std::vector<std::unique_ptr<widget>> _children;
    weak_or_unique_ptr<delegate_type> _delegate;

    using const_iterator = decltype(_children)::const_iterator;

    tab_widget(gui_window &window, widget *parent, weak_or_unique_ptr<delegate_type> delegate) noexcept;
    [[nodiscard]] const_iterator find_selected_child() const noexcept;
    [[nodiscard]] widget &selected_child() const noexcept;
};

} // namespace tt::inline v1
