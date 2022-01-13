// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../widgets/draw_context.hpp"
#include "gfx_surface_state.hpp"
#include "sub_pixel_orientation.hpp"
#include "gfx_system_globals.hpp"

namespace tt::inline v1 {
class gfx_device;
class gfx_system;

class gfx_surface {
public:
    gfx_system &system;

    gfx_surface_state state = gfx_surface_state::no_device;

    /*! Orientation of the RGB subpixels.
     */
    sub_pixel_orientation sub_pixel_orientation = sub_pixel_orientation::BlueRight;
    // sub_pixel_orientation sub_pixel_orientation = sub_pixel_orientation::Unknown;

    virtual ~gfx_surface() {}

    gfx_surface(const gfx_surface &) = delete;
    gfx_surface &operator=(const gfx_surface &) = delete;
    gfx_surface(gfx_surface &&) = delete;
    gfx_surface &operator=(gfx_surface &&) = delete;

    virtual void init() {}

    gfx_surface(gfx_system &system) : system(system) {}

    /*! Set GPU device to manage this window.
     * Change of the device may be done at runtime.
     *
     * @param device The device to use for rendering, may be nullptr.
     */
    virtual void set_device(gfx_device *device) noexcept;

    [[nodiscard]] gfx_device *device() const noexcept
    {
        tt_axiom(gfx_system_mutex.recurse_lock_count());
        return _device;
    }

    /** Get the size of the surface.
     */
    [[nodiscard]] virtual extent2 size() const noexcept = 0;

    void set_closed() noexcept
    {
        ttlet lock = std::scoped_lock(gfx_system_mutex);
        state = gfx_surface_state::window_lost;
    }

    [[nodiscard]] bool is_closed() const noexcept
    {
        ttlet lock = std::scoped_lock(gfx_system_mutex);
        return state == gfx_surface_state::no_window;
    }

    /** Update the surface.
     * This function will check if the graphic pipeline and swapchain
     * needs to be build, rebuild, or torn down.
     *
     * @param new_size The size of the window.
     */
    virtual void update(extent2 new_size) noexcept = 0;

    [[nodiscard]] virtual std::optional<draw_context>
    render_start(aarectangle redraw_rectangle, utc_nanoseconds display_time_point) = 0;
    virtual void render_finish(draw_context const &context, color background_color) = 0;

protected:
    gfx_device *_device = nullptr;

    virtual void teardown() = 0;
};

} // namespace tt::inline v1
