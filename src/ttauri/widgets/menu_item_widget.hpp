// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "abstract_button_widget.hpp"
#include "toolbar_widget.hpp"
#include "../stencils/label_stencil.hpp"
#include "../Path.hpp"
#include "../GUI/draw_context.hpp"
#include <memory>
#include <string>
#include <array>
#include <variant>

namespace tt {

template<typename T>
class menu_item_widget final : public abstract_button_widget<T> {
public:
    using super = abstract_button_widget<T>;
    using value_type = typename super::value_type;

    observable<label> label;

    template<typename Value = observable<value_type>>
    menu_item_widget(gui_window &window, std::shared_ptr<widget> parent, value_type true_value, Value &&value = {}) noexcept :
        super(window, parent, std::move(true_value), std::forward<Value>(value)),
        _parent_is_toolbar(dynamic_cast<toolbar_widget *>(parent.get()) != nullptr)
    {
        // menu item buttons hug the container-border and neighbor widgets.
        this->_margin = 0.0f;
    }

    void init() noexcept override
    {
        super::init();
        _label_callback = this->label.subscribe([this](auto...) {
            this->_request_reconstrain = true;
        });
    }

    /** Set the `show_check_mark()` flag.
     */
    void set_show_check_mark(bool flag) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        this->_show_check_mark = flag;
        this->_request_reconstrain = true;
    }

    /** Whether the label aligns to an optional check-mark.
     * The check-mark denotes that value == true_value.
     *
     * Most menu items, except for the menu items in the toolbar, will want
     * to show a check-mark.
     *
     * @retval true The label is placed after an optional check mark.
     * @retval false The label is flushed with the edge of the menu item.
     */
    [[nodiscard]] bool show_check_mark() const noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        return _show_check_mark;
    }

    /** Set the `show_icon()` flag.
     */
    void set_show_icon(bool flag) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        this->_show_icon = flag;
        this->_request_reconstrain = true;
    }

    /** Whether the text in the label will align to an optional icon in the label.
     * Make space for, and optionally display, an icon in front
     * of the text. This option should be used when any of the labels in a menu
     * has an icon.
     *
     * This should not be used when a menu is displayed in the same direction as
     * the icon label. For example a left or right aligned menu item in a row menu;
     * such as the tool-bar.
     *
     * @retval true The text of the label will be aligned after an optional icon of the label.
     * @retval false The text of the label will be not be aligned to an optional icon of the label.
     */
    [[nodiscard]] bool show_icon() const noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        return _show_icon;
    }

    /** Set the `show_short_cut()` flag.
     */
    void set_show_short_cut(bool flag) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        this->_show_short_cut = flag;
        this->_request_reconstrain = true;
    }

    /** Whether the menu item should make space for an optional short-cut.
     * An optional short-cut may be displayed after the label on the edge of
     * the menu-item.
     *
     * If any of the menu-items in a menu has a short-cut this should be set to
     * true for all of them. This should be set to false for menu items in the tool-bar
     * or for items in a selection widget.
     *
     * @retval true Make room for a short-cut after a label
     * @retval false Don't make extra room for a short-cut after a label.
     */
    [[nodiscard]] bool show_short_cut() const noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        return _show_short_cut;
    }

    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        if (_parent_is_toolbar) {
            return is_toolbar(group) && *this->enabled;
        } else {
            return is_menu(group) && *this->enabled;
        }
    }

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (super::update_constraints(display_time_point, need_reconstrain)) {
            _label_stencil = stencil::make_unique(alignment::middle_left, *label, theme::global->labelStyle);
            _label_stencil->set_show_icon(_show_icon);

            _check_mark_stencil = stencil::make_unique(alignment::middle_center, ElusiveIcon::Ok);

            auto width = _label_stencil->preferred_extent().width() + theme::global->margin * 2.0f;
            if (_show_check_mark) {
                width += theme::global->small_icon_size + theme::global->margin;
            }
            if (_show_short_cut) {
                width += theme::global->margin + theme::global->small_icon_size * 3.0f;
            }

            ttlet height = _label_stencil->preferred_extent().height() + theme::global->margin * 2.0f;
            this->_preferred_size = {
                f32x4{width, height}, f32x4{std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()}};
            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] void update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        need_layout |= std::exchange(this->_request_relayout, false);
        if (need_layout) {
            ttlet check_mark_x = this->rectangle().left() + theme::global->margin;
            ttlet check_mark_width = theme::global->small_icon_size;
            ttlet check_mark_height = theme::global->small_icon_size;
            ttlet check_mark_y = this->rectangle().middle() - check_mark_height * 0.5f;
            ttlet check_mark_rectangle = aarect{check_mark_x, check_mark_y, check_mark_width, check_mark_height};
            _check_mark_stencil->set_layout_parameters(check_mark_rectangle);

            ttlet short_cut_width = theme::global->small_icon_size * 3.0f;
            ttlet short_cut_height = this->rectangle().height();
            ttlet short_cut_x = this->rectangle().right() - theme::global->margin - short_cut_width;
            ttlet short_cut_y = this->rectangle().bottom();
            ttlet short_cut_rectangle = aarect{short_cut_x, short_cut_y, short_cut_width, short_cut_height};

            ttlet label_height = this->rectangle().height();
            ttlet label_y = this->rectangle().bottom();
            auto label_width = this->rectangle().width() - theme::global->margin * 2.0f;
            auto label_x = this->rectangle().left() + theme::global->margin;
            if (_show_check_mark) {
                label_width -= (check_mark_width + theme::global->margin);
                label_x += (check_mark_width + theme::global->margin);
            }
            if (_show_short_cut) {
                label_width -= (theme::global->margin + short_cut_width);
            }
            ttlet label_rectangle = aarect{label_x, label_y, label_width, label_height};

            _label_stencil->set_layout_parameters(label_rectangle);
        }
        super::update_layout(display_time_point, need_layout);
    }

    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (overlaps(context, this->window_clipping_rectangle())) {
            draw_background(context);
            draw_check_mark(context);
            draw_label(context);
        }

        super::draw(std::move(context), display_time_point);
    }

private:
    typename decltype(label)::callback_ptr_type _label_callback;
    std::unique_ptr<label_stencil> _label_stencil;
    std::unique_ptr<image_stencil> _check_mark_stencil;

    bool _parent_is_toolbar;
    bool _show_check_mark = false;
    bool _show_icon = false;
    bool _show_short_cut = false;

    void draw_background(draw_context context) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        context.color = context.fill_color;
        if (this->_focus && this->window.active) {
            context.color = theme::global->accentColor;
        }

        context.draw_box_with_border_inside(this->rectangle());
    }

    void draw_label(draw_context context) noexcept
    {
        context.transform = mat::T(0.0f, 0.0f, 0.1f) * context.transform;
        if (*this->enabled) {
            context.color = theme::global->foregroundColor;
        }
        _label_stencil->draw(context);
    }

    void draw_check_mark(draw_context context) noexcept
    {
        if (this->value == this->true_value) {
            context.transform = mat::T(0.0f, 0.0f, 0.1f) * context.transform;
            if (*this->enabled) {
                context.color = theme::global->foregroundColor;
            }
            _check_mark_stencil->draw(context);
        }
    }
};

} // namespace tt
