// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "gui_surface_state.hpp"
#include "draw_context.hpp"
#include "subpixel_orientation.hpp"

namespace tt {
class gui_device;
class gui_system;

class gui_surface {
public:
    gui_system &system;

    gui_surface_state state = gui_surface_state::no_device;

    //! The current size of the surface.
    extent2 size;

    /*! Orientation of the RGB subpixels.
     */
    subpixel_orientation subpixel_orientation = subpixel_orientation::BlueRight;
    // subpixel_orientation subpixel_orientation = subpixel_orientation::Unknown;

    virtual ~gui_surface() {}

    gui_surface(const gui_surface &) = delete;
    gui_surface &operator=(const gui_surface &) = delete;
    gui_surface(gui_surface &&) = delete;
    gui_surface &operator=(gui_surface &&) = delete;

    virtual void init() {}

    gui_surface(gui_system &system) : system(system) {}

    /*! Set GPU device to manage this window.
     * Change of the device may be done at runtime.
     * 
     * @param device The device to use for rendering, may be nullptr.
     */
    void set_device(gui_device *device) noexcept;

    [[nodiscard]] gui_device *device() const noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        return _device;
    }

    void set_closed() noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        state = gui_surface_state::window_lost;
    }

    [[nodiscard]] bool is_closed() const noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        return state == gui_surface_state::no_window;
    }

    /** Update the surface.
     * This function will check if the graphic pipeline and swapchain
     * needs to be build, rebuild, or torn down.
     * 
     * @param minimum_size The minimum size that the surface is allowed to be.
     * @param maximum_size The maximum size that the surface is allowed to be.
     */
    [[nodiscard]] virtual extent2 update(extent2 minimum_size, extent2 maximum_size) noexcept = 0;

    [[nodiscard]] virtual std::optional<draw_context> render_start(aarectangle redraw_rectangle) = 0;
    virtual void render_finish(draw_context const &context, color background_color) = 0;

protected:
    gui_device *_device;

    virtual void teardown() = 0;
};

}
