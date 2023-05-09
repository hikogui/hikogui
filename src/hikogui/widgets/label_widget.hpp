// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/label_widget.hpp Defines label_widget.
 * @ingroup widgets
 */

#pragma once

#include "../GUI/module.hpp"
#include "text_widget.hpp"
#include "icon_widget.hpp"
#include "../geometry/module.hpp"
#include "../layout/grid_layout.hpp"
#include "../label.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace hi { inline namespace v1 {

template<typename Context>
concept label_widget_attribute = forward_of<Context, observer<hi::label>, observer<hi::alignment>>;

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
template<fixed_string Name = "">
class label_widget final : public widget {
public:
    using super = widget;
    constexpr static auto prefix = Name / "label";

    /** The label to display.
     */
    observer<label> label;

    /** How the label and icon are aligned. Different layouts:
     *  - `alignment::top_left`: icon and text are inline with each other, with
     *    the icon in the top-left corner.
     *  - `alignment::top_right`: icon and text are inline with each other, with
     *    the icon in the top-right corner.
     *  - `alignment::middle_left`: icon and text are inline with each other, with
     *    the icon in the middle-left.
     *  - `alignment::middle_right`: icon and text are inline with each other, with
     *    the icon in the middle-right.
     *  - `alignment::bottom_left`: icon and text are inline with each other, with
     *    the icon in the bottom-left.
     *  - `alignment::bottom_right`: icon and text are inline with each other, with
     *    the icon in the bottom-right.
     *  - `alignment::top_center`: Larger icon above the text, both center aligned.
     *  - `alignment::bottom_center`: Larger icon below the text, both center aligned.
     *    used with a `pixmap` icon.
     */
    observer<alignment> alignment = hi::alignment::top_flush();

    /** Construct a label widget.
     *
     * @see `label_widget::alignment`
     * @param parent The parent widget that owns this radio button widget.
     * @param attributes Different attributes used to configure the label widget:
     *                   a `label`, `alignment` or `text_theme`
     */
    label_widget(widget *parent, label_widget_attribute auto&&...attributes) noexcept : label_widget(parent)
    {
        set_attributes(hi_forward(attributes)...);
    }

    /// @privatesection
    [[nodiscard]] generator<widget const&> children(bool include_invisible) const noexcept override
    {
        co_yield *_icon_widget;
        co_yield *_text_widget;
    }

    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        // Resolve as if in left-to-right mode, the grid will flip itself.
        hilet resolved_alignment = resolve(*alignment, true);

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

        hilet icon_size =
            (resolved_alignment == horizontal_alignment::center or resolved_alignment == horizontal_alignment::justified) ?
            theme<prefix>.size(this) :
            extent2i{theme<prefix>.line_height(this), theme<prefix>.line_height(this)};

        _icon_widget->minimum = icon_size;
        _icon_widget->maximum = icon_size;

        for (auto& cell : _grid) {
            cell.set_constraints(cell.value->update_constraints());
        }

        return _grid.constraints(os_settings::left_to_right());
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        if (compare_store(layout, context)) {
            _grid.set_layout(context.shape, theme<prefix>.cap_height(this));
        }

        for (hilet& cell : _grid) {
            cell.value->set_layout(context.transform(cell.shape, 0.0f));
        }
    }

    void draw(widget_draw_context& context) noexcept override
    {
        if (*mode > widget_mode::invisible and overlaps(context, layout)) {
            for (hilet& cell : _grid) {
                cell.value->draw(context);
            }
        }
    }

    [[nodiscard]] hitbox hitbox_test(point2i position) const noexcept override
    {
        hi_axiom(loop::main().on_thread());

        if (*mode > widget_mode::invisible) {
            return _text_widget->hitbox_test_from_parent(position);
        } else {
            return {};
        }
    }
    /// @endprivatesection
private:
    float _icon_size;
    float _inner_margin;

    decltype(label)::callback_token _label_cbt;
    decltype(alignment)::callback_token _alignment_cbt;

    std::unique_ptr<icon_widget<prefix>> _icon_widget;
    std::unique_ptr<text_widget<prefix>> _text_widget;
    grid_layout<widget *> _grid;

    void set_attributes() noexcept {}
    void set_attributes(label_widget_attribute auto&& first, label_widget_attribute auto&&...rest) noexcept
    {
        if constexpr (forward_of<decltype(first), observer<hi::label>>) {
            label = hi_forward(first);
        } else if constexpr (forward_of<decltype(first), observer<hi::alignment>>) {
            alignment = hi_forward(first);
        } else {
            hi_static_no_default();
        }

        set_attributes(hi_forward(rest)...);
    }

    label_widget(widget *parent) noexcept : super(parent)
    {
        mode = widget_mode::select;

        _icon_widget = std::make_unique<icon_widget<prefix>>(this, label.get<"icon">());
        _text_widget = std::make_unique<text_widget<prefix>>(this, label.get<"text">());
        _text_widget->alignment = alignment;
        _text_widget->mode = mode;

        _alignment_cbt = alignment.subscribe([this](auto...) {
            if (alignment == horizontal_alignment::center or alignment == horizontal_alignment::justified) {
                _icon_widget->alignment = hi::alignment::middle_center();
            } else {
                _icon_widget->alignment = *alignment;
            }
        });
        (*_alignment_cbt)(*alignment);
    }
};

}} // namespace hi::v1
