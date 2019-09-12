// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Window.hpp"
#include "PipelineImage_Image.hpp"
#include "TTauri/Draw/PixelMap.hpp"
#include "TTauri/wsRGBA.hpp"
#include "TTauri/pickle.hpp"

namespace TTauri::GUI::PipelineImage {

template<typename... Args>
inline void Backing::loadOrDraw(Window const &window, extent2 const &currentExtent, std::function<ImagePixelMap(std::shared_ptr<GUI::PipelineImage::Image>)> draw_function, Args&&... keyArgs) {
    required_assert(window.device);
    required_assert(currentExtent.width() > 0 && currentExtent.height() > 0);

    clearAndPickleAppend(keyCache, currentExtent, keyArgs...);

    if (futureImage && futureImage->valid()) {
        auto [newImage, newPixelMap] = futureImage->get();

        window.device->imagePipeline->uploadPixmapToAtlas(*newImage, newPixelMap);

        if (newImage->state == GUI::PipelineImage::Image::State::Uploaded) {
            image = newImage;
            futureImage = {};
        }
    }

    if (!window.resizing) {
        if ((image == nullptr || image->key != keyCache) && !futureImage) {
            auto newImage = window.device->imagePipeline->getImage(keyCache, currentExtent);

            switch (newImage->state) {
            case GUI::PipelineImage::Image::State::Uploaded:
                image = newImage;
                break;

            case GUI::PipelineImage::Image::State::Drawing: {
                auto p = std::promise<ImagePixelMap>();
                futureImage = p.get_future();
                p.set_value({image, Draw::PixelMap<wsRGBA>{}});
            } break;

            case GUI::PipelineImage::Image::State::Uninitialized:
                // Try and draw the image, multiple calls will be dropped by the callee.
                futureImage = std::async([=]() -> ImagePixelMap {
                    // Make sure only one thread will call draw_function().
                    auto expected = GUI::PipelineImage::Image::State::Uninitialized;
                    if (!newImage->state.compare_exchange_strong(expected, GUI::PipelineImage::Image::State::Drawing)) {
                        // Another thread has started drawing.
                        return {newImage, Draw::PixelMap<wsRGBA>{}};
                    }

                    return draw_function(newImage);
                });
                break;
            }
        }
    }
}

}