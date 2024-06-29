// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/label_widget.hpp Defines label_widget.
 * @ingroup widgets
 */

#pragma once

#include "widget.hpp"
#include "text_widget.hpp"
#include "icon_widget.hpp"
#include "../geometry/geometry.hpp"
#include "../layout/layout.hpp"
#include "../l10n/l10n.hpp"
#include "../macros.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>
#include <coroutine>

hi_export_module(hikogui.widgets.label_widget);

hi_export namespace hi { inline namespace v1 {

template<typename Context>
concept label_widget_attribute = forward_of<Context, observer<hi::label>>;

/** The GUI widget displays and lays out text together with an icon.
 * @ingroup widgets
 *
 * This widget is often used by other widgets. For example
 * checkboxes display a label representing their state next
 * to the checkbox itself.
 *
 * The alignment of icon and text is shown in the following image:
 * @image html label_widget.png
 *
 * Here is an example on how to create a label:
 * @snippet widgets/checkbox_example_impl.cpp Create a label
 */
class label_widget : public widget {
public:
    using super = widget;

    /** The label to display.
     */
    observer<label> label;

    /** The color of the label's (non-color) icon.
     */
    observer<hi::phrasing> phrasing = hi::phrasing::regular;

    label_widget() noexcept : super()
    {
        set_mode(widget_mode::select);

        _icon_widget = std::make_unique<icon_widget>(label.sub<"icon">());
        _icon_widget->set_parent(this);
        _icon_widget->phrasing = phrasing;

        _text_widget = std::make_unique<text_widget>(label.sub<"text">());
        _text_widget->set_parent(this);
        _text_widget->set_mode(mode());

        style.set_name("label");
    }

    /** Construct a label widget.
     *
     * @see `label_widget::alignment`
     * @param parent The parent widget that owns this radio button widget.
     * @param attributes Different attributes used to configure the label widget:
     *                   a `label`, `alignment` or `text_style`
     */
    template<label_widget_attribute... Attributes>
    label_widget(Attributes&&...attributes) noexcept : label_widget()
    {
        set_attributes(std::forward<Attributes>(attributes)...);
    }

    /// @privatesection
    [[nodiscard]] generator<widget_intf &> children(bool include_invisible) noexcept override
    {
        co_yield *_icon_widget;
        co_yield *_text_widget;
    }

    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        _layout = {};

        // Resolve as if in left-to-right mode, the grid will flip itself.
        auto const resolved_alignment = resolve(style.alignment, true);

        _grid.clear();
        if (to_bool(label->icon) and to_bool(label->text)) {
            // Both of the icon and text are set, so configure the grid to hold both.
            if (resolved_alignment == horizontal_alignment::left) {
                // icon text
                _grid.add_cell(0, 0, _icon_widget.get());
                _grid.add_cell(1, 0, _text_widget.get(), true);
            } else if (resolved_alignment == horizontal_alignment::right) {
                // text icon
                _grid.add_cell(0, 0, _text_widget.get(), true);
                _grid.add_cell(1, 0, _icon_widget.get());
            } else if (resolved_alignment == vertical_alignment::top) {
                // icon
                // text
                _grid.add_cell(0, 0, _icon_widget.get());
                _grid.add_cell(0, 1, _text_widget.get(), true);
            } else if (resolved_alignment == vertical_alignment::bottom) {
                // text
                // icon
                _grid.add_cell(0, 0, _text_widget.get(), true);
                _grid.add_cell(0, 1, _icon_widget.get());
            } else {
                hi_no_default("alignment is not allowed to be middle-center.");
            }
        } else if (to_bool(label->icon)) {
            // Only the icon-widget is used.
            _grid.add_cell(0, 0, _icon_widget.get());
        } else if (to_bool(label->text)) {
            // Only the text-widget is used.
            _grid.add_cell(0, 0, _text_widget.get());
        }

        auto const icon_size = style.font_size_px;
        _icon_widget->minimum = extent2{icon_size, icon_size};
        _icon_widget->maximum = extent2{icon_size, icon_size};

        for (auto& cell : _grid) {
            cell.set_constraints(cell.value->update_constraints());
        }

        return _grid.constraints(os_settings::left_to_right());
    }
    void set_layout(widget_layout const& context) noexcept override
    {
        if (compare_store(_layout, context)) {
            _grid.set_layout(context.shape);
        }

        for (auto const& cell : _grid) {
            cell.value->set_layout(context.transform(cell.shape, transform_command::level));
        }
    }
    
    void draw(draw_context const& context) noexcept override
    {
        if (mode() > widget_mode::invisible and overlaps(context, layout())) {
            for (auto const& cell : _grid) {
                cell.value->draw(context);
            }
        }
    }

    [[nodiscard]] hitbox hitbox_test(point2 position) const noexcept override
    {
        hi_axiom(loop::main().on_thread());

        if (mode() > widget_mode::invisible) {
            return _text_widget->hitbox_test_from_parent(position);
        } else {
            return {};
        }
    }
    /// @endprivatesection
private:
    float _icon_size;
    float _inner_margin;

    std::unique_ptr<icon_widget> _icon_widget;
    std::unique_ptr<text_widget> _text_widget;
    grid_layout<widget *> _grid;

    callback<void(hi::label)> _label_cbt;
    callback<void(hi::alignment)> _alignment_cbt;

    void set_attributes() noexcept {}

    template<label_widget_attribute First, label_widget_attribute... Rest>
    void set_attributes(First&& first, Rest&&...rest) noexcept
    {
        if constexpr (forward_of<First, observer<hi::label>>) {
            label = std::forward<First>(first);
        } else if constexpr (forward_of<First, observer<hi::phrasing>>) {
            phrasing = std::forward<First>(first);
        } else {
            hi_static_no_default();
        }

        set_attributes(std::forward<Rest>(rest)...);
    }
};

}} // namespace hi::v1
