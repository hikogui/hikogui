
#pragma once

#include "PipelineImage.hpp"

namespace TTauri::GUI {

struct PipelineImage::Image {
    bool drawn = false;

    u16vec2 extent;
    std::vector<uint16_t> atlasIndicies;

    //std::pair<gsl::span<uint32_t>, u16vec2> beginCPUDrawing();
    //void finishCPUDrawing();

};

}