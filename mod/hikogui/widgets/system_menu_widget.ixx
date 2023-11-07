// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/system_menu_widget.hpp Defines system_menu_widget.
 * @ingroup widgets
 */

module;
#include "../macros.hpp"

#include <memory>
#include <string>
#include <array>
#include <coroutine>

export module hikogui_widgets_system_menu_widget;
import hikogui_coroutine;
import hikogui_l10n;
import hikogui_widgets_icon_widget;
import hikogui_widgets_widget;

export namespace hi { inline namespace v1 {

/** The system menu widget.
 * This widget displays an icon in the menu bar of the window and is used to call-up
 * the operating-system supplied menu to control the window.
 *
 * @ingroup widgets
 */
class system_menu_widget final : public widget {
public:
    using super = widget;

    observer<icon> icon;

    ~system_menu_widget() {}

    system_menu_widget(not_null<widget_intf const *> parent) noexcept : super(parent)
    {
        _icon_widget = std::make_unique<icon_widget>(this, icon);
    }

    system_menu_widget(not_null<widget_intf const *> parent, forward_of<observer<hi::icon>> auto&& icon) noexcept :
        system_menu_widget(parent)
    {
        this->icon = hi_forward(icon);
    }

    /// @privatesection
    [[nodiscard]] generator<widget_intf &> children(bool include_invisible) noexcept override
    {
        co_yield *_icon_widget;
    }

    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        hi_assert_not_null(_icon_widget);

        _layout = {};
        _icon_constraints = _icon_widget->update_constraints();

        hilet size = extent2{theme().large_size(), theme().large_size()};
        return {size, size, size};
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        if (compare_store(_layout, context)) {
            hilet icon_height =
                context.height() < round_cast<int>(theme().large_size() * 1.2f) ? context.height() : theme().large_size();
            hilet icon_rectangle = aarectangle{0, context.height() - icon_height, context.width(), icon_height};
            _icon_shape = box_shape{_icon_constraints, icon_rectangle, theme().baseline_adjustment()};
            // Leave space for window resize handles on the left and top.
            _system_menu_rectangle = aarectangle{
                theme().margin<float>(),
                0.0f,
                context.width() - theme().margin<float>(),
                context.height() - theme().margin<float>()};
        }

        _icon_widget->set_layout(context.transform(_icon_shape));
    }

    void draw(draw_context const& context) noexcept override
    {
        if (*mode > widget_mode::invisible and overlaps(context, layout())) {
            _icon_widget->draw(context);
        }
    }

    [[nodiscard]] hitbox hitbox_test(point2 position) const noexcept override
    {
        hi_axiom(loop::main().on_thread());

        if (*mode >= widget_mode::partial and layout().contains(position)) {
            // Only the top-left square should return ApplicationIcon, leave
            // the reset to the toolbar implementation.
            return {id, _layout.elevation, hitbox_type::application_icon};
        } else {
            return {};
        }
    }

    /// @endprivatesection
private:
    std::unique_ptr<icon_widget> _icon_widget;
    box_constraints _icon_constraints;
    box_shape _icon_shape;

    aarectangle _system_menu_rectangle;
};

}} // namespace hi::v1
