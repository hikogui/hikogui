// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/hires_utc_clock.hpp"
#include "TTauri/Foundation/vec.hpp"
#include "TTauri/GUI/HitBox.hpp"
#include "TTauri/GUI/Window_forward.hpp"
#include <rhea/variable.hpp>
#include <rhea/linear_expression.hpp>
#include <functional>

namespace TTauri::GUI::Widgets {
class Widget;
}

namespace TTauri::GUI {
struct MouseEvent;
struct KeyboardEvent;
class DrawContext;

inline std::function<void(Widgets::Widget *)> Widget_delete;

struct WidgetDeleter {
    void operator()(Widgets::Widget *ptr) const noexcept {
        Widget_delete(ptr);
    }
};

inline std::function<std::unique_ptr<Widgets::Widget,WidgetDeleter>(Window &)> WindowWidget_makeUnique;

inline std::function<int(Widgets::Widget &, hires_utc_clock::time_point)> Widget_needs;

inline std::function<void(Widgets::Widget &, hires_utc_clock::time_point)> Widget_layout;
inline std::function<void(Widgets::Widget &, DrawContext const &, hires_utc_clock::time_point)> Widget_draw;

inline std::function<int(Widgets::Widget &, hires_utc_clock::time_point, bool)> Widget_layoutChildren;

inline std::function<Widgets::Widget *(Widgets::Widget const &)> Widget_getNextKeyboardWidget;
inline std::function<Widgets::Widget *(Widgets::Widget const &)> Widget_getPreviousKeyboardWidget;
inline std::function<bool(Widgets::Widget const &)> Widget_acceptsFocus;

inline std::function<void(Widgets::Widget &, MouseEvent const &)> Widget_handleMouseEvent;
inline std::function<void(Widgets::Widget &, KeyboardEvent const &)> Widget_handleKeyboardEvent;
inline std::function<vec(Widgets::Widget const &)> Widget_getWindowOffset;
inline std::function<HitBox(Widgets::Widget const &, vec)> Widget_hitBoxTest;

inline std::function<rhea::variable const &(Widgets::Widget const &)> Widget_getLeft;
inline std::function<rhea::variable const &(Widgets::Widget const &)> Widget_getBottom;
inline std::function<rhea::variable const &(Widgets::Widget const &)> Widget_getWidth;
inline std::function<rhea::variable const &(Widgets::Widget const &)> Widget_getHeight;
inline std::function<rhea::linear_expression const &(Widgets::Widget const &)> Widget_getRight;
inline std::function<rhea::linear_expression const &(Widgets::Widget const &)> Widget_getTop;


}
