// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/Window.hpp"
#include "TTauri/GUI/PipelineMSDF_Image.hpp"
#include "TTauri/Foundation/PixelMap.hpp"
#include "TTauri/Foundation/pickle.hpp"
#include "TTauri/Foundation/wsRGBA.hpp"

namespace TTauri::GUI::PipelineMSDF {

template<typename... Args>
inline void Backing::loadOrDraw(Window const &window, extent2 const &currentExtent, std::function<ImagePixelMap(std::shared_ptr<GUI::PipelineMSDF::Image>)> draw_function, Args&&... keyArgs) {
    ttauri_assert(window.device);
    ttauri_assert(currentExtent.width() > 0 && currentExtent.height() > 0);

    clearAndPickleAppend(keyCache, currentExtent, keyArgs...);

    if (futureImage && futureImage->valid()) {
        auto [newImage, newPixelMap] = futureImage->get();

        window.device->imagePipeline->uploadPixmapToAtlas(*newImage, newPixelMap);

        if (newImage->state == GUI::PipelineMSDF::Image::State::Uploaded) {
            image = newImage;
            futureImage = {};
        }
    }

    if (!window.resizing) {
        if ((image == nullptr || image->key != keyCache) && !futureImage) {
            auto newImage = window.device->imagePipeline->getImage(keyCache, currentExtent);

            switch (newImage->state) {
            case GUI::PipelineMSDF::Image::State::Uploaded:
                image = newImage;
                break;

            case GUI::PipelineMSDF::Image::State::Drawing: {
                auto p = std::promise<ImagePixelMap>();
                futureImage = p.get_future();
                p.set_value({image, PixelMap<wsRGBA>{}});
            } break;

            case GUI::PipelineMSDF::Image::State::Uninitialized:
                // Try and draw the image, multiple calls will be dropped by the callee.
                futureImage = std::async([=]() -> ImagePixelMap {
                    // Make sure only one thread will call draw_function().
                    auto expected = GUI::PipelineMSDF::Image::State::Uninitialized;
                    if (!newImage->state.compare_exchange_strong(expected, GUI::PipelineMSDF::Image::State::Drawing)) {
                        // Another thread has started drawing.
                        return {newImage, PixelMap<wsRGBA>{}};
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