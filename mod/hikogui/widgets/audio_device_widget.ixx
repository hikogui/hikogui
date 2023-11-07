// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/audio_device_widget.hpp Defines audio_device_widget.
 * @ingroup widgets
 */

module;
#include "../macros.hpp"

#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>
#include <coroutine>

export module hikogui_widgets_audio_device_widget;
import hikogui_audio;
import hikogui_coroutine;
import hikogui_l10n;
import hikogui_widgets_grid_widget;
import hikogui_widgets_selection_widget;

export namespace hi { inline namespace v1 {

/** Audio device configuration widget.
 * @ingroup widgets
 */
class audio_device_widget final : public widget {
public:
    using super = widget;

    /** The audio device this widget has selected and is configuring.
     */
    observer<std::string> device_id;

    /** The audio direction (input or output) of devices is should show.
     */
    observer<audio_direction> direction = audio_direction::bidirectional;

    virtual ~audio_device_widget() {}

    audio_device_widget(not_null<widget_intf const *> parent) noexcept : super(parent)
    {
        _grid_widget = std::make_unique<grid_widget>(this);
        _device_selection_widget = &_grid_widget->emplace<selection_widget>("A1", device_id, _device_list);

        _sync_device_list_task = sync_device_list();
    }

    /// @privatesection
    [[nodiscard]] generator<widget_intf&> children(bool include_invisible) noexcept override
    {
        co_yield *_grid_widget;
    }

    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        _layout = {};
        _grid_constraints = _grid_widget->update_constraints();
        return _grid_constraints;
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        if (compare_store(_layout, context)) {
            hilet grid_rectangle = context.rectangle();
            _grid_shape = {_grid_constraints, grid_rectangle, theme().baseline_adjustment()};
        }

        _grid_widget->set_layout(context.transform(_grid_shape, transform_command::level));
    }

    void draw(draw_context const& context) noexcept override
    {
        if (*mode > widget_mode::invisible) {
            _grid_widget->draw(context);
        }
    }

    hitbox hitbox_test(point2 position) const noexcept override
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
    /** The grid widget contains all the child widgets.
     */
    std::unique_ptr<grid_widget> _grid_widget;
    box_constraints _grid_constraints;
    box_shape _grid_shape;

    /** The widget used to select the audio device.
     */
    selection_widget *_device_selection_widget = nullptr;

    observer<std::vector<std::pair<std::string, label>>> _device_list;

    hi::scoped_task<> _sync_device_list_task;

    [[nodiscard]] hi::scoped_task<> sync_device_list() noexcept
    {
        while (true) {
            {
                auto proxy = _device_list.copy();
                proxy->clear();
                for (auto& device : audio_devices(hi::audio_device_state::active, *direction)) {
                    proxy->emplace_back(device.id(), device.label());
                }
            }

            co_await when_any(audio_system::global(), direction);
        }
    }
};

}} // namespace hi::v1
