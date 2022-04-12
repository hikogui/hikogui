// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <memory>

namespace hi::inline v1 {

class RenderDoc {
public:
    RenderDoc() noexcept;

    void set_overlay(bool frameRate, bool frameNumber, bool captureList) noexcept;

private:
    /** Pointer to the RenderDoc API struct.
     */
    void *api = nullptr;
};

} // namespace hi::inline v1
