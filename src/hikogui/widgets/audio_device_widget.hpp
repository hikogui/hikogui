// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/audio_device_widget.hpp Defines audio_device_widget.
 * @ingroup widgets
 */

#pragma once

#include "selection_widget.hpp"
#include "grid_widget.hpp"
#include "../GUI/module.hpp"
#include "../audio/audio_system.hpp"
#include "../audio/audio_device.hpp"
#include "../audio/audio_direction.hpp"
#include "../label.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace hi { inline namespace v1 {

/** Audio device configuration widget.
 * @ingroup widgets
 */
template<fixed_string Name = "">
class audio_device_widget final : public widget {
public:
    using super = widget;
    constexpr static auto prefix = Name / "audio-device";

    /** The audio device this widget has selected and is configuring.
     */
    observer<std::string> device_id;

    /** The audio direction (input or output) of devices is should show.
     */
    observer<audio_direction> direction = audio_direction::bidirectional;

    audio_device_widget(widget *parent, hi::audio_system& audio_system) noexcept :
        super(parent), _audio_system(&audio_system)
    {
        _grid_widget = std::make_unique<grid_widget<prefix>>(this);
        _device_selection_widget = &_grid_widget->make_widget<selection_widget<prefix>>("A1", device_id, _device_list);

        _sync_device_list_task = sync_device_list();
    }

    /// @privatesection
    [[nodiscard]] generator<widget const&> children(bool include_invisible) const noexcept override
    {
        co_yield *_grid_widget;
    }

    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        _grid_constraints = _grid_widget->update_constraints();
        return _grid_constraints;
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        if (compare_store(layout, context)) {
            hilet grid_rectangle = context.rectangle();
            _grid_shape = {_grid_constraints, grid_rectangle, theme<prefix>.int_cap_height(this)};
        }

        _grid_widget->set_layout(context.transform(_grid_shape));
    }

    void draw(widget_draw_context const& context) noexcept override
    {
        if (*mode > widget_mode::invisible) {
            _grid_widget->draw(context);
        }
    }

    hitbox hitbox_test(point2i position) const noexcept override
    {
        if (*mode >= widget_mode::partial) {
            auto r = hitbox{};
            r = _grid_widget->hitbox_test_from_parent(position, r);
            return r;
        } else {
            return hitbox{};
        }
    }

    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override
    {
        if (*mode >= widget_mode::partial) {
            return _grid_widget->accepts_keyboard_focus(group);
        } else {
            return false;
        }
    }

    /// @endprivatesection
private:
    hi::audio_system *_audio_system;

    /** The grid widget contains all the child widgets.
     */
    std::unique_ptr<grid_widget<prefix>> _grid_widget;
    box_constraints _grid_constraints;
    box_shape _grid_shape;

    /** The widget used to select the audio device.
     */
    selection_widget<prefix> *_device_selection_widget = nullptr;

    observer<std::vector<std::pair<std::string, label>>> _device_list;

    hi::scoped_task<> _sync_device_list_task;

    [[nodiscard]] hi::scoped_task<> sync_device_list() noexcept
    {
        while (true) {
            {
                auto proxy = _device_list.copy();
                proxy->clear();
                for (auto& device : _audio_system->devices()) {
                    if (device.state() == hi::audio_device_state::active and to_bool(device.direction() & *direction)) {
                        proxy->emplace_back(device.id(), device.label());
                    }
                }
            }

            co_await when_any(*_audio_system, direction);
        }
    }
};

}} // namespace hi::v1
