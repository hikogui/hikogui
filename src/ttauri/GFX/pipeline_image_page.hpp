// Copyright Take Vos 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../required.hpp"
#include <limits>

namespace tt::pipeline_image {

/** Page of an image
 *
 * The page of an image is a square part of an image.
 */
struct page {
    static constexpr size_t size = 64;
    static constexpr size_t border = 1;

    size_t nr;

    /** Create a page on the texture atlas
     *
     * @param nr The page number identifies a specific page in the texture atlas.
     */
    constexpr page(size_t nr) : nr(nr) {}

    /** Create a transparent page.
     */
    constexpr page() : nr(std::numeric_limits<size_t>::max()) {}

    bool is_fully_transparent() const noexcept
    {
        return nr == std::numeric_limits<size_t>::max();
    }
};

} // namespace tt::pipeline_image
