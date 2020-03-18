// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/Window.hpp"
#include "TTauri/GUI/PipelineImage_Image.hpp"
#include "TTauri/Foundation/PixelMap.hpp"
#include "TTauri/Foundation/pickle.hpp"

namespace TTauri::GUI::PipelineImage {

template<typename... Args>
inline void Backing::loadOrDraw(Window const &window, vec const &currentExtent, std::function<ImagePixelMap(std::shared_ptr<GUI::PipelineImage::Image>)> draw_function, Args&&... keyArgs) {
    ttauri_assert(window.device);
    ttauri_assert(currentExtent.x() > 0 && currentExtent.y() > 0);

    clearAndPickleAppend(keyCache, currentExtent, keyArgs...);

    if (futureImage && futureImage->valid()) {
        auto [newImage, newPixelMap] = futureImage->get();

        // Only uploads when the state is in ::Drawing and newPixelMap != zero
        // and switches to ::Uploading once it finishes.
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
                    p.set_value({newImage, PixelMap<R16G16B16A16SFloat>{}});
                } break;

            case GUI::PipelineImage::Image::State::Uninitialized:
                // Try and draw the image, multiple calls will be dropped by the callee.
                futureImage = std::async([=]() -> ImagePixelMap {
                    // Make sure only one thread will call draw_function().
                    auto expected = GUI::PipelineImage::Image::State::Uninitialized;
                    if (!newImage->state.compare_exchange_strong(expected, GUI::PipelineImage::Image::State::Drawing)) {
                        // Another thread has started drawing.
                        return {newImage, PixelMap<R16G16B16A16SFloat>{}};
                    }

                    return draw_function(newImage);
                });
#if !defined(NDEBUG)
                // Synchronize to make debugging easier.
                (*futureImage).wait();
#endif
                break;
            }
        }
    }
}
}