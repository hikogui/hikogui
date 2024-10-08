
#pragma once

#include "../macros.hpp"

hi_export_module(hikogui.image : image_loader);

hi_export namespace hi::inline v1 {

/** Load an image from a file.
 * 
 * If the filename ends with `<name>@<scale>x.<ext>` then the image has a
 * scale that is used to match the current screen density.
 * 
 * The scaler has the following format:
 *  - none: The image is meant to be displayed at 1:1 scale on a 72 ppi screen.
 *  - @2x: The image is meant to be displayed at 1:1 scale on a 144 ppi screen.
 * 
 * 
 * @param path The path to the image file.
 * @param density The pixel density of the screen.
 * @return The image.
 */
[[nodiscard]] std::expected<pixmap<sfloat_rgba16>, std::error_code> load_image(std::filesystem::path const& path, hi::pixel_density density) noexcept;

/** Load in image from a file.
 * 
 * The image loaded is specified by the search parameters in the bookmark.
 * The pixel density is used to determine the most optimal image to load, when
 * multiple versions of the image are available.
 * 
 * @param bm The bookmark to the image.
 * @param density The pixel density of the screen.
 * @return The image.
 */
[[nodiscard]] std::expected<pixmap<sfloat_rgba16>, std::error_code> load_image(hi::bookmark const& bm, hi::pixel_density density) noexcept;

}
