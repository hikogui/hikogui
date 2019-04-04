#include "PipelineImage_Image.hpp"

namespace TTauri::GUI {
TTauri::Draw::PixelMap<uint32_t> PipelineImage::Image::beginCPUDrawing()
{
    return TTauri::Draw::PixelMap<uint32_t>{gsl::span<uint32_t>{}, 0, 0};
}

}
