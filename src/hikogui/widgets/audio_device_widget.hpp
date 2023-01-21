// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/audio_device_widget.hpp Defines audio_device_widget.
 * @ingroup widgets
 */

#pragma once

#include "selection_widget.hpp"
#include "grid_widget.hpp"
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
class audio_device_widget final : public widget {
public:
    using super = widget;

    /** The audio device this widget has selected and is configuring.
     */
    observer<std::string> device_id;

    /** The audio direction (input or output) of devices is should show.
     */
    observer<audio_direction> direction = audio_direction::bidirectional;

    virtual ~audio_device_widget();

    audio_device_widget(widget *parent, hi::audio_system& audio_system) noexcept;

    /// @privatesection
    [[nodiscard]] generator<widget const &> children(bool include_invisible) const noexcept override;
    [[nodiscard]] box_constraints update_constraints() noexcept override;
    void set_layout(widget_layout const& context) noexcept override;
    void draw(draw_context const& context) noexcept override;
    hitbox hitbox_test(point2i position) const noexcept override;
    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override;
    /// @endprivatesection
private:
    hi::audio_system *_audio_system;

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

    [[nodiscard]] hi::scoped_task<> sync_device_list() noexcept;
};

}} // namespace hi::v1
