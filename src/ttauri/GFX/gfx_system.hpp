// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "gfx_device.hpp"
#include "../unfair_recursive_mutex.hpp"
#include "../subsystem.hpp"
#include <span>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace tt {
class gfx_surface;
class font_book;

/** Graphics system
 */
class gfx_system {
public:
    //! List of all devices.
    std::vector<std::shared_ptr<gfx_device>> devices;

    /** The font book to request glyphs from.
    * 
    * The font_book is owned by the graphics system because
    * draw operations for glyphs is handled by the graphics system.
    * 
    * The GUI system and its widgets will also need the font book
    * to handle text shaping. In this case the GUI system will request
    * a reference from the graphics system.
    */
    std::unique_ptr<tt::font_book> font_book;

    gfx_system(std::unique_ptr<tt::font_book> font_book) noexcept;

    virtual ~gfx_system();

    gfx_system(const gfx_system &) = delete;
    gfx_system &operator=(const gfx_system &) = delete;
    gfx_system(gfx_system &&) = delete;
    gfx_system &operator=(gfx_system &&) = delete;

    /** Initialize after construction.
     * Call this function directly after the constructor on the same thread.
     */
    virtual void init() {};
    virtual void deinit() {};

    [[nodiscard]] virtual std::unique_ptr<gfx_surface> make_surface(os_handle instance, void *os_window) const noexcept = 0;

    gfx_device *find_best_device_for_surface(gfx_surface const &surface);
};

}
