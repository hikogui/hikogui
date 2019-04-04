
#pragma once

#include "PipelineImage.hpp"
#include "TTauri/Draw/PixelMap.hpp"

namespace TTauri::GUI {

struct PipelineImage::Image {
    bool drawn = false;

    u16vec2 extent;
    std::vector<uint16_t> atlasIndicies;

    TTauri::Draw::PixelMap<uint32_t> beginCPUDrawing();
    //void finishCPUDrawing();

};

}