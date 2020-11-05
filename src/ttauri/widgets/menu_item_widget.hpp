// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "abstract_button_widget.hpp"
#include "../stencils/label_stencil.hpp"
#include "../Path.hpp"
#include "../GUI/DrawContext.hpp"
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
    menu_item_widget(Window &window, std::shared_ptr<widget> parent, value_type true_value, Value &&value = {}) noexcept :
        super(window, parent, std::move(true_value), std::forward<Value>(value))
    {
        // Toolbar buttons hug the toolbar and neighbor widgets.
        this->_margin = 0.0f;
    }

    void initialize() noexcept override
    {
        super::initialize();
        _label_callback = this->label.subscribe([this](auto...) {
            this->_request_reconstrain = true;
        });
    }

    /** Set the `show_check_mark()` flag.
     */
    void set_show_check_mark(bool flag) noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());
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
        tt_assume(GUISystem_mutex.recurse_lock_count());
        return _show_check_mark;
    }

    /** Set the `show_icon()` flag.
     */
    void set_show_icon(bool flag) noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());
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
        tt_assume(GUISystem_mutex.recurse_lock_count());
        return _show_icon;
    }

    /** Set the `show_short_cut()` flag.
     */
    void set_show_short_cut(bool flag) noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());
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
        tt_assume(GUISystem_mutex.recurse_lock_count());
        return _show_short_cut;
    }

    [[nodiscard]] bool update_constraints() noexcept override
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        if (super::update_constraints()) {
            _label_stencil = stencil::make_unique(Alignment::MiddleLeft, *label, theme->labelStyle);
            _label_stencil->set_show_icon(_show_icon);

            _check_mark_stencil = stencil::make_unique(Alignment::MiddleCenter, ElusiveIcon::Ok);

            auto width = _label_stencil->preferred_extent().width() + Theme::margin * 2.0f;
            if (_show_check_mark) {
                width += Theme::small_icon_size + Theme::margin;
            }
            if (_show_short_cut) {
                width += Theme::margin + Theme::small_icon_size * 3.0f;
            }

            ttlet height = _label_stencil->preferred_extent().height() + Theme::margin * 2.0f;
            this->_preferred_size = {
                vec{width, height}, vec{std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()}};
            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] bool update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        need_layout |= std::exchange(this->_request_relayout, false);
        if (need_layout) {
            ttlet check_mark_x = this->rectangle().left() + Theme::margin;
            ttlet check_mark_width = Theme::small_icon_size;
            ttlet check_mark_height = Theme::small_icon_size;
            ttlet check_mark_y = this->rectangle().middle() - check_mark_height * 0.5f;
            ttlet check_mark_rectangle = aarect{check_mark_x, check_mark_y, check_mark_width, check_mark_height};
            _check_mark_stencil->set_layout_parameters(check_mark_rectangle);

            ttlet short_cut_width = Theme::small_icon_size * 3.0f;
            ttlet short_cut_height = this->rectangle().height();
            ttlet short_cut_x = this->rectangle().right() - Theme::margin - short_cut_width;
            ttlet short_cut_y = this->rectangle().bottom();
            ttlet short_cut_rectangle = aarect{short_cut_x, short_cut_y, short_cut_width, short_cut_height};

            ttlet label_height = this->rectangle().height();
            ttlet label_y = this->rectangle().bottom();
            auto label_width = this->rectangle().width() - Theme::margin * 2.0f;
            auto label_x = this->rectangle().left() + Theme::margin;
            if (_show_check_mark) {
                label_width -= (check_mark_width + Theme::margin);
                label_x += (check_mark_width + Theme::margin);
            }
            if (_show_short_cut) {
                label_width -= (Theme::margin + short_cut_width);
            }
            ttlet label_rectangle = aarect{label_x, label_y, label_width, label_height};

            _label_stencil->set_layout_parameters(label_rectangle);
        }
        return super::update_layout(display_time_point, need_layout);
    }

    void draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());
        draw_background(context);
        draw_check_mark(context);
        draw_label(context);
        super::draw(std::move(context), display_time_point);
    }

private:
    typename decltype(label)::callback_ptr_type _label_callback;
    std::unique_ptr<label_stencil> _label_stencil;
    std::unique_ptr<image_stencil> _check_mark_stencil;

    bool _show_check_mark = false;
    bool _show_icon = false;
    bool _show_short_cut = false;

    void draw_background(DrawContext context) noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        context.color = context.fillColor;
        if (this->_focus && this->window.active) {
            context.color = theme->accentColor;
        }

        context.drawBoxIncludeBorder(this->rectangle());
    }

    void draw_label(DrawContext context) noexcept
    {
        context.transform = mat::T(0.0f, 0.0f, 0.1f) * context.transform;
        if (*this->enabled) {
            context.color = theme->foregroundColor;
        }
        _label_stencil->draw(context);
    }

    void draw_check_mark(DrawContext context) noexcept
    {
        if (this->value == this->true_value) {
            context.transform = mat::T(0.0f, 0.0f, 0.1f) * context.transform;
            if (*this->enabled) {
                context.color = theme->foregroundColor;
            }
            _check_mark_stencil->draw(context);
        }
    }
};

} // namespace tt
