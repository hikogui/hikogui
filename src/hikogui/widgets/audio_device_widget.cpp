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
            for (auto device : _audio_system->devices()) {
                if (device->state() == hi::audio_device_state::active and to_bool(device->direction() & *direction)) {
                    proxy->emplace_back(device->id(), device->label());
                }
            }
        }

        co_await when_any(*_audio_system, direction);
    }
}

[[nodiscard]] generator<widget *> audio_device_widget::children() const noexcept{
    co_yield _grid_widget.get();
}

widget_constraints const& audio_device_widget::set_constraints(set_constraints_context const& context) noexcept
{
    _layout = {};
    _constraints = _grid_widget->set_constraints(context);
    // The device_selection_widget will have a very strong baseline,
    // the parent widget will likely conform and the calculation during layout
    // will yield the same absolute baseline.
    _constraints.baseline = _device_selection_widget->constraints().baseline;
    return _constraints;
}

void audio_device_widget::set_layout(widget_layout const& context) noexcept
{
    if (compare_store(_layout, context)) {
        _grid_rectangle = context.rectangle();
    }

    _grid_widget->set_layout(context.transform(_grid_rectangle));
}

void audio_device_widget::draw(draw_context const& context) noexcept
{
    if (*mode > widget_mode::invisible) {
        _grid_widget->draw(context);
    }
}

hitbox audio_device_widget::hitbox_test(point3 position) const noexcept
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
