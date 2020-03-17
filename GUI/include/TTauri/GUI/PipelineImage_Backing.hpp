// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/Window_forward.hpp"
#include <future>
#include <map>
#include <optional>
#include <memory>
#include <string>

namespace TTauri {
struct wsRGBA;
}

namespace TTauri {
template<typename T> struct PixelMap;
}

namespace TTauri::GUI::PipelineImage {
struct Image;

/*! A backing image for widgets.
 * It contains an PipelineImage::Image which is a reference to the TextureAtlas to be used to send vertices to the pipeline.
 */
struct Backing {
    using ImagePixelMap = std::pair<std::shared_ptr<GUI::PipelineImage::Image>,PixelMap<wsRGBA>>;

    /*! A reference to the texture-atlas to be used to send vertices to the pipeline.
     */
    std::shared_ptr<GUI::PipelineImage::Image> image;

    /*! An optional future image and pixelmap to be uploaded to the atlas
     * waiting to replace the `image`.
     */
    std::optional<std::future<ImagePixelMap>> futureImage;

    /*! The key to request a reference to the atlas.
     * This cache will be overwritten with each call to `loadOrDraw()` but it reduces the
     * amount of allocations being done for each render call.
     */
    std::string keyCache;

    /*! This should be called on each render call to potentially update the image when the key changes.
     * This function will make sure that:
     *  * draw_function() will only be called when the window is not being resized.
     *  * That for a state change the draw_function() is only called once.
     *  * That multiple state changes will not cause overlapping calls to draw_function().
     */
    template<typename... Args>
    void loadOrDraw(Window const &window, vec const &currentExtent, std::function<ImagePixelMap(std::shared_ptr<GUI::PipelineImage::Image>)> draw_function, Args&&... keyArgs);
};

}
