// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace hi::inline v1 {

class gfx_surface_delegate {
public:
    virtual ~gfx_surface_delegate() = default;
    constexpr gfx_surface_delegate() noexcept = default;
    gfx_surface_delegate(gfx_surface_delegate const&) = delete;
    gfx_surface_delegate(gfx_surface_delegate&&) = delete;
    gfx_surface_delegate &operator=(gfx_surface_delegate const&) = delete;
    gfx_surface_delegate &operator=(gfx_surface_delegate&&) = delete;


};


}
