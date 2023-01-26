// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "audio_device_widget.hpp"
#include "../when_any.hpp"

namespace hi::inline v1 {

audio_device_widget::~audio_device_widget() {}

audio_device_widget::audio_device_widget(widget *parent, hi::audio_system& audio_system) noexcept :
    super(parent), _audio_system(&audio_system)
{
    _grid_widget = std::make_unique<grid_widget>(this);
    _device_selection_widget = &_grid_widget->make_widget<selection_widget>("A1", device_id, _device_list);

    _sync_device_list_task = sync_device_list();
}

[[nodiscard]] hi::scoped_task<> audio_device_widget::sync_device_list() noexcept
{
    while (true) {
        {
            auto proxy = _device_list.copy();
            proxy->clear();
            for (auto &device : _audio_system->devices()) {
                if (device.state() == hi::audio_device_state::active and to_bool(device.direction() & *direction)) {
                    proxy->emplace_back(device.id(), device.label());
                }
            }
        }

        co_await when_any(*_audio_system, direction);
    }
}

[[nodiscard]] generator<widget const&> audio_device_widget::children(bool include_invisible) const noexcept
{
    co_yield *_grid_widget;
}

[[nodiscard]] box_constraints audio_device_widget::update_constraints() noexcept
{
    _layout = {};
    _grid_constraints = _grid_widget->update_constraints();
    return _grid_constraints;
}

void audio_device_widget::set_layout(widget_layout const& context) noexcept
{
    if (compare_store(_layout, context)) {
        hilet grid_rectangle = context.rectangle();
        _grid_shape = {_grid_constraints, grid_rectangle, theme().baseline_adjustment()};
    }

    _grid_widget->set_layout(context.transform(_grid_shape));
}

void audio_device_widget::draw(draw_context const& context) noexcept
{
    if (*mode > widget_mode::invisible) {
        _grid_widget->draw(context);
    }
}

hitbox audio_device_widget::hitbox_test(point2i position) const noexcept
{
    if (*mode >= widget_mode::partial) {
        auto r = hitbox{};
        r = _grid_widget->hitbox_test_from_parent(position, r);
        return r;
    } else {
        return hitbox{};
    }
}

[[nodiscard]] bool audio_device_widget::accepts_keyboard_focus(keyboard_focus_group group) const noexcept
{
    if (*mode >= widget_mode::partial) {
        return _grid_widget->accepts_keyboard_focus(group);
    } else {
        return false;
    }
}

} // namespace hi::inline v1
