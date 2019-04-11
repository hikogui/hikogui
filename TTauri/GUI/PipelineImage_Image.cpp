#include "PipelineImage_Image.hpp"
#include "PipelineImage_DeviceShared.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

namespace TTauri::GUI {

u64rect PipelineImage::Image::indexToRect(size_t const sliceIndex) const
{
    auto const indexY = sliceIndex / sliceExtent.x;
    auto const indexX = sliceIndex % sliceExtent.x;

    auto const left = indexX * PipelineImage::DeviceShared::atlasSliceWidth;
    auto const top = indexY * PipelineImage::DeviceShared::atlasSliceHeight;
    auto const right = left + PipelineImage::DeviceShared::atlasSliceWidth;
    auto const bottom = top + PipelineImage::DeviceShared::atlasSliceHeight;
    auto const rightOverflow = right - std::min(right, static_cast<size_t>(extent.x));
    auto const bottomOverflow = bottom - std::min(bottom, static_cast<size_t>(extent.y));
    auto const width = PipelineImage::DeviceShared::atlasSliceWidth - rightOverflow;
    auto const height = PipelineImage::DeviceShared::atlasSliceHeight - bottomOverflow;

    return {{left, top}, {width, height}};
}

void PipelineImage::Image::placeSliceVertices(size_t const index, const ImageLocation &location, gsl::span<PipelineImage::Vertex> &vertices, size_t &offset) const {
    auto const slice = slices.at(index);

    if (slice == std::numeric_limits<uint16_t>::max()) {
        // Hole in the image does not need to be rendered.
        return;
    }

    auto const rect = indexToRect(index);

    // Calculate position of each vertex of this index within the image.
    auto lt = glm::vec2{ rect.offset.x, rect.offset.y };
    auto rt = lt + glm::vec2{ rect.extent.x, 0.0 };
    auto lb = lt + glm::vec2{ 0.0, rect.extent.y };
    auto rb = lt + glm::vec2{ rect.extent.x, rect.extent.y };

    // Calculate positions compared to origin of the image.
    lt -= location.origin;
    rt -= location.origin;
    lb -= location.origin;
    rb -= location.origin;

    // Rotate positions around the origin.
    lt = glm::rotate(lt, location.rotation);
    rt = glm::rotate(rt, location.rotation);
    lb = glm::rotate(lb, location.rotation);
    rb = glm::rotate(rb, location.rotation);

    // Move positions to window coordinates.
    lt += location.position;
    rt += location.position;
    lb += location.position;
    rb += location.position;
    
    // Drop slice when fully outside of clipping rectangle
    auto const clip_low_x = location.clippingRectangle.offset.x;
    auto const clip_low_y = location.clippingRectangle.offset.y;
    auto const clip_high_x = location.clippingRectangle.offset.x + location.clippingRectangle.extent.x;
    auto const clip_high_y = location.clippingRectangle.offset.y + location.clippingRectangle.extent.y;

    if (lt.x < clip_low_x && rt.x < clip_low_x && lb.x < clip_low_x && rt.x < clip_low_x) { return; }
    if (lt.x > clip_high_x && rt.x > clip_high_x && lb.x > clip_high_x && rt.x > clip_high_x) { return; }
    if (lt.y < clip_low_y && rt.y < clip_low_y && lb.y < clip_low_y && rt.y < clip_low_y) { return; }
    if (lt.y > clip_high_y && rt.y > clip_high_y && lb.y > clip_high_y && rt.y > clip_high_y) { return; }

    auto const atlasPosition = PipelineImage::DeviceShared::getAtlasPositionFromSlice(slice);

    auto &v_lt = vertices.at(offset++);
    v_lt.position = lt;
    v_lt.atlasPosition = atlasPosition;
    v_lt.clippingRectangle = location.clippingRectangle;
    v_lt.depth = location.depth;
    v_lt.alpha = location.alpha;

    auto &v_rt = vertices.at(offset++);
    v_rt.position = rt;
    v_rt.atlasPosition = {atlasPosition.x + rect.extent.x, atlasPosition.y, atlasPosition.z};
    v_rt.clippingRectangle = location.clippingRectangle;
    v_rt.depth = location.depth;
    v_rt.alpha = location.alpha;

    auto &v_lb = vertices.at(offset++);
    v_lb.position = lb;
    v_lb.atlasPosition = {atlasPosition.x, atlasPosition.y + rect.extent.y, atlasPosition.z};
    v_lb.clippingRectangle = location.clippingRectangle;
    v_lb.depth = location.depth;
    v_lb.alpha = location.alpha;

    auto &v_rb = vertices.at(offset++);
    v_rb.position = rb;
    v_rb.atlasPosition = {atlasPosition.x + rect.extent.x, atlasPosition.y + rect.extent.y, atlasPosition.z};
    v_rb.clippingRectangle = location.clippingRectangle;
    v_rb.depth = location.depth;
    v_rb.alpha = location.alpha;
}

/*! Place vertices for this image.
* An image is build out of atlas slices, that need to be individual rendered.
* A slice with the value std::numeric_limits<uint16_t>::max() is not rendered.
*
* \param position Position (x, y) from the left-top of the window in pixels. Z equals depth.
* \param origin Origin (x, y) from the left-top of the image in pixels. Z equals rotation clockwise around the origin in radials.
*/
void PipelineImage::Image::placeVertices(const ImageLocation &location, gsl::span<PipelineImage::Vertex> &vertices, size_t &offset) const {
    for (size_t index = 0; index < slices.size(); index++) {
        placeSliceVertices(index, location, vertices, offset);
    }
}

}
