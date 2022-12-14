// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "draw_context.hpp"
#include "gfx_surface_state.hpp"
#include "gfx_system_globals.hpp"

namespace hi::inline v1 {
class gfx_device;
class gfx_system;
class gfx_surface_delegate;

class gfx_surface {
public:
    gfx_system &system;

    gfx_surface_state state = gfx_surface_state::has_window;
    gfx_surface_loss loss = gfx_surface_loss::none;

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
        hi_axiom(gfx_system_mutex.recurse_lock_count());
        return _device;
    }

    /** Get the size of the surface.
     */
    [[nodiscard]] virtual extent2i size() const noexcept = 0;

    /** Update the surface.
     * This function will check if the graphic pipeline and swapchain
     * needs to be build, rebuild, or torn down.
     *
     * @param new_size The size of the window.
     */
    virtual void update(extent2i new_size) noexcept = 0;

    [[nodiscard]] virtual draw_context render_start(aarectanglei redraw_rectangle) = 0;
    virtual void render_finish(draw_context const &context) = 0;

    /** Add a delegate to handle extra rendering.
    * 
    * The delegate can render underneath the HikoGUI user interface.
    */
    virtual void add_delegate(gfx_surface_delegate *delegate) noexcept = 0;

    /** Remove a delegate
     */
    virtual void remove_delegate(gfx_surface_delegate *delegate) noexcept = 0;

protected:
    gfx_device *_device = nullptr;

    virtual void teardown() noexcept = 0;
};

} // namespace hi::inline v1
