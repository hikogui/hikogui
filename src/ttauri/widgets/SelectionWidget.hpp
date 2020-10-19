// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"
#include "WindowWidget.hpp"
#include "../cells/TextCell.hpp"
#include "../GUI/DrawContext.hpp"
#include "../text/FontBook.hpp"
#include "../text/format10.hpp"
#include "../text/ElusiveIcons.hpp"
#include "../observable.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

template<typename ValueType>
class SelectionWidget final : public Widget {
public:
    using option_list_type = observable<std::vector<std::pair<ValueType, std::u8string>>>;
    using value_type = observable<ValueType>;
    using label_type = observable<std::u8string>;

    label_type label;
    value_type value;
    option_list_type option_list;

    SelectionWidget(
        Window &window,
        Widget *parent,
        value_type const &value = {},
        option_list_type const &option_list = {},
        label_type const &label = l10n(u8"<unknown>")) noexcept :
        Widget(window, parent), value(value), option_list(option_list), label(label)
    {
        [[maybe_unused]] ttlet value_cbid = this->value.add_callback([this](auto...) {
            this->window.requestRedraw = true;
        });
        [[maybe_unused]] ttlet option_list_cbid = this->option_list.add_callback([this](auto...) {
            request_reconstrain = true;
        });
        [[maybe_unused]] ttlet label_cbid = this->label.add_callback([this](auto...) {
            request_reconstrain = true;
        });
    }

    template<typename... Args>
    SelectionWidget(
        Window &window,
        Widget *parent,
        value_type const &value,
        option_list_type const &option_list,
        l10n const &fmt,
        Args const &... args) noexcept :
        SelectionWidget(window, parent, value, option_list, format(fmt, args...))
    {
    }

    template<typename... Args>
    SelectionWidget(
        Window &window,
        Widget *parent,
        option_list_type const &option_list,
        l10n const &fmt,
        Args const &... args) noexcept :
        SelectionWidget(window, parent, value_type{}, option_list, format(fmt, args...))
    {
    }

    template<typename... Args>
    SelectionWidget(Window &window, Widget *parent, value_type const &value, l10n const &fmt, Args const &... args) noexcept :
        SelectionWidget(window, parent, value, option_list_type{}, format(fmt, args...))
    {
    }

    template<typename... Args>
    SelectionWidget(Window &window, Widget *parent, l10n const &fmt, Args const &... args) noexcept :
        SelectionWidget(window, parent, value_type{}, option_list_type{}, format(fmt, args...))
    {
    }

    ~SelectionWidget() {}

    [[nodiscard]] bool update_constraints() noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        if (Widget::update_constraints()) {
            labelCell = std::make_unique<TextCell>(*label, theme->placeholderLabelStyle);
            auto preferredWidth = labelCell->preferredExtent().width();
            auto preferredHeight = labelCell->preferredExtent().height();

            // Create a list of cells, one for each option and calculate
            // the optionHeight based on the option which is the tallest.
            option_cache_list.clear();
            for (ttlet & [ tag, labelText ] : *option_list) {
                auto cell = std::make_unique<TextCell>(labelText, theme->labelStyle);
                preferredWidth = std::max(preferredWidth, cell->preferredExtent().width());
                preferredHeight = std::max(preferredHeight, cell->preferredExtent().height());
                option_cache_list.emplace_back(tag, std::move(cell));
            }

            // Set the widget height to the tallest option, fallback to a small size widget.
            if (preferredHeight == 0.0f) {
                preferredHeight = Theme::smallSize;
            }

            p_preferred_size = interval_vec2::make_minimum(
                Theme::smallSize + preferredWidth + Theme::margin * 2.0f, preferredHeight + Theme::margin * 2.0f);
            p_preferred_base_line = relative_base_line{VerticalAlignment::Middle, 0.0f, 200.0f};

            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] bool update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        need_layout |= std::exchange(request_relayout, false);
        if (need_layout) {
            leftBoxRectangle = aarect{0.0f, 0.0f, Theme::smallSize, rectangle().height()};

            ttlet optionX = leftBoxRectangle.p3().x() + Theme::margin;
            ttlet optionWidth = rectangle().width() - optionX - Theme::margin;
            ttlet optionHeight = rectangle().height() - Theme::margin * 2.0f;

            // Calculate the rectangles for each cell in the option_cache_list
            option_cache_list_height = (optionHeight + Theme::margin * 2.0f) * std::ssize(option_cache_list);
            ttlet option_cache_listWidth = optionWidth + Theme::margin * 2.0f;
            auto y = option_cache_list_height;
            selectedOptionY = 0.0f;
            for (auto &&option : option_cache_list) {
                y -= Theme::margin;
                y -= optionHeight;
                option.cellRectangle = aarect{Theme::margin, y, optionWidth, optionHeight};
                option.backgroundRectangle = expand(option.cellRectangle, Theme::margin);
                y -= Theme::margin;

                if (option.tag == value) {
                    selectedOptionY = y;
                }
            }

            // The window height, excluding the top window decoration.
            ttlet windowHeight = window.widget->rectangle().height() - Theme::toolbarHeight;

            // Calculate overlay dimensions and position.
            ttlet overlayWidth = option_cache_listWidth;
            ttlet overlayWindowX = p_window_rectangle.x() + Theme::smallSize;
            ttlet overlayHeight = std::min(option_cache_list_height, windowHeight);
            auto overlayWindowY = p_window_rectangle.y() - selectedOptionY;

            // Adjust overlay to fit inside the window, below the top window decoration.
            overlayWindowY = std::clamp(overlayWindowY, 0.0f, windowHeight - overlayHeight);

            overlayWindowRectangle = aarect{overlayWindowX, overlayWindowY, overlayWidth, overlayHeight};

            // The overlayRectangle are in the coordinate system of the current widget, so it will
            // extent beyond the current widget.
            overlayRectangle =
                aarect{overlayWindowX - p_window_rectangle.x(), overlayWindowY - p_window_rectangle.y(), overlayWidth, overlayHeight};

            // The label is located to the right of the selection box icon.
            optionRectangle = aarect{optionX, rectangle().height() - optionHeight - Theme::margin, optionWidth, optionHeight};

            chevronsGlyph = to_FontGlyphIDs(ElusiveIcon::ChevronUp);
            ttlet chevronsGlyphBB = PipelineSDF::DeviceShared::getBoundingBox(chevronsGlyph);
            chevronsRectangle = align(leftBoxRectangle, scale(chevronsGlyphBB, Theme::iconSize), Alignment::MiddleCenter);
        }
        return Widget::update_layout(display_time_point, need_layout);
    }

    void draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        drawOutline(context);
        drawLeftBox(context);
        drawChevrons(context);
        drawValue(context);

        if (selecting) {
            drawOverlay(context);
        }

        Widget::draw(std::move(context), display_time_point);
    }

    bool handle_keyboard_event(KeyboardEvent const &event) noexcept override
    {
        ttlet lock = std::scoped_lock(mutex);
        auto handled = Widget::handle_keyboard_event(event);

        if (event.type == KeyboardEvent::Type::Exited) {
            handled = true;
            handle_command(command::gui_escape);
        }

        return handled;
    }

    bool handle_mouse_event(MouseEvent const &event) noexcept override
    {
        ttlet lock = std::scoped_lock(mutex);
        auto handled = Widget::handle_mouse_event(event);

        if (selecting) {
            ttlet position = from_window_transform * event.position;

            auto mouseInListPosition = mat::T{-overlayRectangle.x(), -overlayRectangle.y()} * position;

            if (overlayRectangle.contains(position)) {
                for (ttlet &option : option_cache_list) {
                    if (option.backgroundRectangle.contains(mouseInListPosition)) {
                        if (hoverOption != option.tag) {
                            window.requestRedraw = true;
                            hoverOption = option.tag;
                        }
                    }
                }

            } else {
                if (hoverOption.has_value()) {
                    window.requestRedraw = true;
                    hoverOption = {};
                }
            }
        }

        if (event.cause.leftButton) {
            handled = true;
            if (*enabled) {
                if (selecting) {
                    if (event.type == MouseEvent::Type::ButtonDown) {
                        clickedOption = hoverOption;
                        window.requestRedraw = true;
                    }

                    if (event.type == MouseEvent::Type::ButtonUp) {
                        if (clickedOption.has_value() && clickedOption == hoverOption) {
                            chosenOption = *clickedOption;
                            handle_command(command::gui_activate);
                        }
                        clickedOption = {};
                        window.requestRedraw = true;
                    }

                } else if (event.type == MouseEvent::Type::ButtonUp && p_window_rectangle.contains(event.position)) {
                    handle_command(command::gui_activate);
                }
            }
        }
        return handled;
    }

    bool handle_command(command command) noexcept override
    {
        ttlet lock = std::scoped_lock(mutex);
        auto handled = Widget::handle_command(command);

        if (*enabled) {
            switch (command) {
            case command::gui_up: {
                handled = true;
                std::optional<ValueType> prev_tag;
                for (ttlet &option : option_cache_list) {
                    if (option.tag == chosenOption && prev_tag.has_value()) {
                        chosenOption = *prev_tag;
                        break;
                    }
                    prev_tag = option.tag;
                }
                } break;

            case command::gui_down: {
                handled = true;
                bool found = false;
                for (ttlet &option : option_cache_list) {
                    if (found) {
                        chosenOption = option.tag;
                        break;
                    } else if (option.tag == chosenOption) {
                        found = true;
                    }
                }
                } break;

            case command::gui_activate:
                handled = true;
                if (selecting) {
                    selecting = false;
                    value.store(chosenOption);

                } else {
                    selecting = true;
                    chosenOption = value.load();
                }
                break;

            case command::gui_widget_next:
            case command::gui_widget_prev:
                // We are not handling these commands, just committing the chosen option.
                if (selecting) {
                    selecting = false;
                    value.store(chosenOption);
                }
                break;

            case command::gui_escape:
                handled = true;
                if (selecting) {
                    selecting = false;
                }
                break;

            default:;
            }
        }

        request_relayout = true;
        return handled;
    }

    [[nodiscard]] HitBox hitbox_test(vec window_position) const noexcept override
    {
        ttlet lock = std::scoped_lock(mutex);
        ttlet position = from_window_transform * window_position;

        if (selecting && overlayRectangle.contains(position)) {
            return HitBox{this, p_draw_layer + 25.0f, *enabled ? HitBox::Type::Button : HitBox::Type::Default};

        } else if (p_window_clipping_rectangle.contains(window_position)) {
            return HitBox{this, p_draw_layer, *enabled ? HitBox::Type::Button : HitBox::Type::Default};

        } else {
            return HitBox{};
        }
    }

    [[nodiscard]] bool accepts_focus() const noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());
        return *enabled;
    }

private:
    std::unique_ptr<TextCell> labelCell;

    struct option_list_entry {
        ValueType tag;
        std::unique_ptr<TextCell> cell;
        aarect cellRectangle;
        aarect backgroundRectangle;

        option_list_entry(ValueType tag, std::unique_ptr<TextCell> cell) noexcept :
            tag(tag), cell(std::move(cell)), cellRectangle(), backgroundRectangle()
        {
        }
    };

    float option_cache_list_height;
    std::vector<option_list_entry> option_cache_list;
    float selectedOptionY;

    aarect optionRectangle;
    aarect leftBoxRectangle;

    FontGlyphIDs chevronsGlyph;
    aarect chevronsRectangle;

    aarect overlayWindowRectangle;
    aarect overlayRectangle;

    bool selecting = false;
    ValueType chosenOption;
    std::optional<ValueType> hoverOption;
    std::optional<ValueType> clickedOption;

    void drawOptionHighlight(DrawContext drawContext, option_list_entry const &option) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        drawContext.transform = mat::T{0.0, 0.0, 0.1f} * drawContext.transform;

        if (option.tag == chosenOption && !clickedOption.has_value()) {
            drawContext.fillColor = theme->accentColor;
        } else if (option.tag == clickedOption) {
            drawContext.fillColor = theme->accentColor;
        } else if (option.tag == hoverOption) {
            drawContext.fillColor = theme->fillColor(p_semantic_layer + 1);
        } else {
            drawContext.fillColor = theme->fillColor(p_semantic_layer);
        }
        drawContext.drawFilledQuad(option.backgroundRectangle);
    }

    void drawOptionLabel(DrawContext drawContext, option_list_entry const &option) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        drawContext.transform = mat::T{0.0, 0.0, 0.3f} * drawContext.transform;
        drawContext.color = theme->labelStyle.color;
        option.cell->draw(drawContext, option.cellRectangle, Alignment::MiddleLeft, center(option.cellRectangle).y(), true);
    }

    void drawOption(DrawContext drawContext, option_list_entry const &option) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        drawOptionHighlight(drawContext, option);
        drawOptionLabel(drawContext, option);
    }

    void drawOverlayOutline(DrawContext drawContext) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        drawContext.transform = mat::T{0.0, 0.0, 0.1f} * drawContext.transform;
        drawContext.fillColor = drawContext.fillColor.a(0.0f);
        drawContext.drawBoxIncludeBorder(overlayRectangle);
    }

    void drawOverlay(DrawContext context) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        context.transform = mat::T{0.0, 0.0, 25.0f} * context.transform;
        context.clippingRectangle = expand(overlayWindowRectangle, Theme::borderWidth * 0.5f);

        drawOverlayOutline(context);

        auto option_cache_listContext = context;
        option_cache_listContext.transform = mat::T{overlayRectangle.x(), overlayRectangle.y()} * context.transform;
        for (ttlet &option : option_cache_list) {
            drawOption(option_cache_listContext, option);
        }
    }

    void drawOutline(DrawContext drawContext) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        drawContext.cornerShapes = Theme::roundingRadius;
        drawContext.drawBoxIncludeBorder(rectangle());
    }

    void drawLeftBox(DrawContext drawContext) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        drawContext.transform = mat::T{0.0, 0.0, 0.1f} * drawContext.transform;
        // if (*enabled && window.active) {
        //    drawContext.color = theme->accentColor;
        //}
        drawContext.fillColor = drawContext.color;
        drawContext.cornerShapes = vec{Theme::roundingRadius, 0.0f, Theme::roundingRadius, 0.0f};
        drawContext.drawBoxIncludeBorder(leftBoxRectangle);
    }

    void drawChevrons(DrawContext drawContext) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        drawContext.transform = mat::T{0.0, 0.0, 0.2f} * drawContext.transform;
        drawContext.color = *enabled ? theme->foregroundColor : drawContext.fillColor;
        drawContext.drawGlyph(chevronsGlyph, chevronsRectangle);
    }

    void drawValue(DrawContext drawContext) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        auto i = std::find_if(option_cache_list.cbegin(), option_cache_list.cend(), [this](ttlet &item) {
            return item.tag == value;
        });

        if (i != option_cache_list.cend()) {
            drawContext.transform = mat::T{0.0, 0.0, 0.1f} * drawContext.transform;
            drawContext.color = *enabled ? theme->labelStyle.color : drawContext.color;
            i->cell->draw(drawContext, optionRectangle, Alignment::MiddleLeft, base_line(), true);
        } else {
            drawContext.transform = mat::T{0.0, 0.0, 0.1f} * drawContext.transform;
            drawContext.color = *enabled ? theme->placeholderLabelStyle.color : drawContext.color;
            labelCell->draw(drawContext, optionRectangle, Alignment::MiddleLeft, base_line(), true);
        }
    }
};

} // namespace tt
