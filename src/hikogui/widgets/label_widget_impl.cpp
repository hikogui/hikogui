// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "label_widget.hpp"

namespace hi::inline v1 {

label_widget::label_widget(widget *parent) noexcept : super(parent)
{
    mode = widget_mode::select;

    _icon_widget = std::make_unique<icon_widget>(this, label.get<"icon">());
    _text_widget = std::make_unique<text_widget>(this, label.get<"text">());
    _text_widget->alignment = alignment;
    _text_widget->text_style = text_style;
    _text_widget->mode = mode;

    _alignment_cbt = alignment.subscribe([this](auto...) {
        if (alignment == horizontal_alignment::center or alignment == horizontal_alignment::justified) {
            _icon_widget->alignment = hi::alignment::middle_center();
        } else {
            _icon_widget->alignment = *alignment;
        }
    });
    (*_alignment_cbt)(*alignment);

    _text_style_cbt = text_style.subscribe(
        [this](auto...) {
            switch (*text_style) {
            case semantic_text_style::label:
                _icon_widget->color = color::foreground();
                break;
            case semantic_text_style::small_label:
                _icon_widget->color = color::foreground();
                break;
            case semantic_text_style::warning:
                _icon_widget->color = color::orange();
                break;
            case semantic_text_style::error:
                _icon_widget->color = color::red();
                break;
            case semantic_text_style::help:
                _icon_widget->color = color::indigo();
                break;
            case semantic_text_style::placeholder:
                _icon_widget->color = color::gray();
                break;
            case semantic_text_style::link:
                _icon_widget->color = color::blue();
                break;
            default:
                _icon_widget->color = color::foreground();
            }
        });
}

[[nodiscard]] box_constraints label_widget::update_constraints() noexcept
{
    _layout = {};

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
        theme().large_icon_size() :
        narrow_cast<int>(std::ceil(theme().text_style(*text_style)->size * theme().scale));

    _icon_widget->minimum = extent2i{icon_size, icon_size};
    _icon_widget->maximum = extent2i{icon_size, icon_size};

    for (auto& cell : _grid) {
        cell.set_constraints(cell.value->update_constraints());
    }

    return _grid.constraints(os_settings::left_to_right());
}

void label_widget::set_layout(widget_layout const& context) noexcept
{
    if (compare_store(_layout, context)) {
        _grid.set_layout(context.shape, theme().baseline_adjustment());
    }

    for (hilet& cell : _grid) {
        cell.value->set_layout(context.transform(cell.shape, 0.0f));
    }
}

void label_widget::draw(draw_context const& context) noexcept
{
    if (*mode > widget_mode::invisible and overlaps(context, layout())) {
        for (hilet& cell : _grid) {
            cell.value->draw(context);
        }
    }
}

[[nodiscard]] hitbox label_widget::hitbox_test(point2i position) const noexcept
{
    hi_axiom(loop::main().on_thread());

    if (*mode > widget_mode::invisible) {
        return _text_widget->hitbox_test_from_parent(position);
    } else {
        return {};
    }
}

} // namespace hi::inline v1
