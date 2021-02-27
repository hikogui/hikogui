// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "pipeline_SDF.hpp"
#include "pipeline_SDF_device_shared.hpp"
#include "gui_device_vulkan.hpp"
#include "../text/shaped_text.hpp"
#include "../pixel_map.hpp"
#include "../URL.hpp"
#include "../memory.hpp"
#include "../cast.hpp"
#include "../numeric_array.hpp"
#include "../aarect.hpp"
#include "../geometry/scale.hpp"
#include "../geometry/translate.hpp"
#include <array>

namespace tt::pipeline_SDF {

using namespace std;

device_shared::device_shared(gui_device_vulkan const &device) : device(device)
{
    buildShaders();
    buildAtlas();
}

device_shared::~device_shared() {}

void device_shared::destroy(gui_device_vulkan *vulkanDevice)
{
    tt_axiom(vulkanDevice);

    teardownShaders(vulkanDevice);
    teardownAtlas(vulkanDevice);
}

[[nodiscard]] atlas_rect device_shared::allocateRect(extent2 drawExtent) noexcept
{
    auto imageWidth = narrow_cast<int>(std::ceil(drawExtent.width()));
    auto imageHeight = narrow_cast<int>(std::ceil(drawExtent.height()));

    if (atlas_allocation_position.y() + imageHeight > atlasImageHeight) {
        atlas_allocation_position.x() = 0;
        atlas_allocation_position.y() = 0;
        atlas_allocation_position.z() = atlas_allocation_position.z() + 1;

        if (atlas_allocation_position.z() >= atlasMaximumNrImages) {
            tt_log_fatal("pipeline_SDF atlas overflow, too many glyphs in use.");
        }

        if (atlas_allocation_position.z() >= size(atlasTextures)) {
            addAtlasImage();
        }
    }

    if (atlas_allocation_position.x() + imageWidth > atlasImageWidth) {
        atlas_allocation_position.x() = 0;
        atlas_allocation_position.y() = atlas_allocation_position.y() + atlasAllocationMaxHeight;
    }

    auto r = atlas_rect{atlas_allocation_position, drawExtent};

    atlas_allocation_position.x() = atlas_allocation_position.x() + imageWidth;
    atlasAllocationMaxHeight = std::max(atlasAllocationMaxHeight, imageHeight);

    return r;
}

void device_shared::uploadStagingPixmapToAtlas(atlas_rect location)
{
    // Flush the given image, included the border.
    device.flushAllocation(
        stagingTexture.allocation, 0, (stagingTexture.pixel_map.height() * stagingTexture.pixel_map.stride()) * sizeof(sdf_r8));

    stagingTexture.transitionLayout(device, vk::Format::eR8Snorm, vk::ImageLayout::eTransferSrcOptimal);

    array<vector<vk::ImageCopy>, atlasMaximumNrImages> regionsToCopyPerAtlasTexture;

    auto regionsToCopy = std::vector{vk::ImageCopy{
        {vk::ImageAspectFlagBits::eColor, 0, 0, 1},
        {0, 0, 0},
        {vk::ImageAspectFlagBits::eColor, 0, 0, 1},
        {narrow_cast<int32_t>(location.atlas_position.x()), narrow_cast<int32_t>(location.atlas_position.y()), 0},
        {narrow_cast<uint32_t>(location.size.width()), narrow_cast<uint32_t>(location.size.height()), 1}}};

    auto &atlasTexture = atlasTextures.at(narrow_cast<size_t>(location.atlas_position.z()));
    atlasTexture.transitionLayout(device, vk::Format::eR8Snorm, vk::ImageLayout::eTransferDstOptimal);

    device.copyImage(
        stagingTexture.image,
        vk::ImageLayout::eTransferSrcOptimal,
        atlasTexture.image,
        vk::ImageLayout::eTransferDstOptimal,
        std::move(regionsToCopy));
}

void device_shared::prepareStagingPixmapForDrawing()
{
    stagingTexture.transitionLayout(device, vk::Format::eR8Snorm, vk::ImageLayout::eGeneral);
}

void device_shared::prepareAtlasForRendering()
{
    for (auto &atlasTexture : atlasTextures) {
        atlasTexture.transitionLayout(device, vk::Format::eR8Snorm, vk::ImageLayout::eShaderReadOnlyOptimal);
    }
}

/** Prepare the atlas for drawing a text.
 *
 *  +---------------------+
 *  |     draw border     |
 *  |  +---------------+  |
 *  |  | render border |  |
 *  |  |  +---------+  |  |
 *  |  |  |  glyph  |  |  |
 *  |  |  | bounding|  |  |
 *  |  |  |   box   |  |  |
 *  |  |  +---------+  |  |
 *  |  |               |  |
 *  |  +---------------+  |
 *  |                     |
 *  O---------------------+
 */
atlas_rect device_shared::addGlyphToAtlas(font_glyph_ids glyph) noexcept
{
    ttlet[glyphPath, glyphBoundingBox] = glyph.getPathAndBoundingBox();

    ttlet drawScale = scale2{drawfontSize, drawfontSize};
    ttlet scaledBoundingBox = drawScale * glyphBoundingBox;

    // We will draw the font at a fixed size into the texture. And we need a border for the texture to
    // allow proper bi-linear interpolation on the edges.

    // Determine the size of the image in the atlas.
    // This is the bounding box sized to the fixed font size and a border
    ttlet drawOffset = vector2{drawBorder, drawBorder} - scaledBoundingBox.offset();
    ttlet drawExtent = scaledBoundingBox.extent() + 2.0f * drawBorder;
    ttlet drawTranslate = translate2{drawOffset};

    // Transform the path to the scale of the fixed font size and drawing the bounding box inside the image.
    ttlet drawPath = (drawTranslate * drawScale) * glyphPath;

    // Draw glyphs into staging buffer of the atlas and upload it to the correct position in the atlas.
    prepareStagingPixmapForDrawing();
    auto atlas_rect = allocateRect(drawExtent);
    auto pixmap = stagingTexture.pixel_map.submap(aarect{atlas_rect.size});
    fill(pixmap, drawPath);
    uploadStagingPixmapToAtlas(atlas_rect);

    return atlas_rect;
}

std::pair<atlas_rect, bool> device_shared::getGlyphFromAtlas(font_glyph_ids glyph) noexcept
{
    ttlet i = glyphs_in_atlas.find(glyph);
    if (i != glyphs_in_atlas.cend()) {
        return {i->second, false};

    } else {
        ttlet aarect = addGlyphToAtlas(glyph);
        glyphs_in_atlas.emplace(glyph, aarect);
        return {aarect, true};
    }
}

aarect device_shared::getBoundingBox(font_glyph_ids const &glyphs) noexcept
{
    // Adjust bounding box by adding a border based on 1EM.
    return expand(glyphs.getBoundingBox(), scaledDrawBorder);
}

bool device_shared::_place_vertices(
    vspan<vertex> &vertices,
    aarect clipping_rectangle,
    rect box,
    font_glyph_ids const &glyphs,
    color color) noexcept
{
    ttlet[atlas_rect, glyph_was_added] = getGlyphFromAtlas(glyphs);

    ttlet p0 = get<0>(box);
    ttlet p1 = get<1>(box);
    ttlet p2 = get<2>(box);
    ttlet p3 = get<3>(box);

    // If none of the vertices is inside the clipping rectangle then don't add the
    // quad to the vertex list.
    if (!overlaps(clipping_rectangle, aarect{box})) {
        return glyph_was_added;
    }

    vertices.emplace_back(p0, clipping_rectangle, get<0>(atlas_rect.texture_coordinates), color);
    vertices.emplace_back(p1, clipping_rectangle, get<1>(atlas_rect.texture_coordinates), color);
    vertices.emplace_back(p2, clipping_rectangle, get<2>(atlas_rect.texture_coordinates), color);
    vertices.emplace_back(p3, clipping_rectangle, get<3>(atlas_rect.texture_coordinates), color);
    return glyph_was_added;
}

bool device_shared::_place_vertices(
    vspan<vertex> &vertices,
    aarect clipping_rectangle,
    matrix3 transform,
    attributed_glyph const &attr_glyph,
    color color) noexcept
{
    if (!is_visible(attr_glyph.general_category)) {
        return false;
    }

    // Adjust bounding box by adding a border based on 1EM.
    ttlet bounding_box = transform * attr_glyph.boundingBox(scaledDrawBorder);

    return _place_vertices(vertices, clipping_rectangle, bounding_box, attr_glyph.glyphs, color);
}

bool device_shared::_place_vertices(
    vspan<vertex> &vertices,
    aarect clipping_rectangle,
    matrix3 transform,
    attributed_glyph const &attr_glyph
    ) noexcept
{
    return _place_vertices(vertices, clipping_rectangle, transform, attr_glyph, attr_glyph.style.color);
}

void device_shared::place_vertices(
    vspan<vertex> &vertices,
    aarect clippingRectangle,
    rect box,
    font_glyph_ids const &glyphs,
    color color
    ) noexcept
{
    if (_place_vertices(vertices, clippingRectangle, box, glyphs, color)) {
        prepareAtlasForRendering();
    }
}

void device_shared::place_vertices(
    vspan<vertex> &vertices,
    aarect clipping_rectangle,
    matrix3 transform,
    shaped_text const &text
    ) noexcept
{
    auto atlas_was_updated = false;

    for (ttlet &attr_glyph : text) {
        ttlet glyph_added = _place_vertices(vertices, clipping_rectangle, transform, attr_glyph);
        atlas_was_updated = atlas_was_updated || glyph_added;
    }

    if (atlas_was_updated) {
        prepareAtlasForRendering();
    }
}

void device_shared::place_vertices(
    vspan<vertex> &vertices,
    aarect clipping_rectangle,
    matrix3 transform,
    shaped_text const &text,
    color color) noexcept
{
    auto atlas_was_updated = false;

    for (ttlet &attr_glyph : text) {
        ttlet glyph_added = _place_vertices(vertices, clipping_rectangle, transform, attr_glyph, color);
        atlas_was_updated = atlas_was_updated || glyph_added;
    }

    if (atlas_was_updated) {
        prepareAtlasForRendering();
    }
}

void device_shared::drawInCommandBuffer(vk::CommandBuffer &commandBuffer)
{
    commandBuffer.bindIndexBuffer(device.quadIndexBuffer, 0, vk::IndexType::eUint16);
}

void device_shared::buildShaders()
{
    specializationConstants.sdf_r8maxDistance = sdf_r8::max_distance;
    specializationConstants.atlasImageWidth = atlasImageWidth;

    fragmentShaderSpecializationMapEntries = specialization_constants::specializationConstantMapEntries();
    fragmentShaderSpecializationInfo = specializationConstants.specializationInfo(fragmentShaderSpecializationMapEntries);

    vertexShaderModule = device.loadShader(URL("resource:GUI/pipeline_SDF.vert.spv"));
    fragmentShaderModule = device.loadShader(URL("resource:GUI/pipeline_SDF.frag.spv"));

    shaderStages = {
        {vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main"},
        {vk::PipelineShaderStageCreateFlags(),
         vk::ShaderStageFlagBits::eFragment,
         fragmentShaderModule,
         "main",
         &fragmentShaderSpecializationInfo}};
}

void device_shared::teardownShaders(gui_device_vulkan *vulkanDevice)
{
    tt_axiom(vulkanDevice);

    vulkanDevice->destroy(vertexShaderModule);
    vulkanDevice->destroy(fragmentShaderModule);
}

void device_shared::addAtlasImage()
{
    // ttlet currentImageIndex = std::ssize(atlasTextures);

    // Create atlas image
    vk::ImageCreateInfo const imageCreateInfo = {
        vk::ImageCreateFlags(),
        vk::ImageType::e2D,
        vk::Format::eR8Snorm,
        vk::Extent3D(atlasImageWidth, atlasImageHeight, 1),
        1, // mipLevels
        1, // arrayLayers
        vk::SampleCountFlagBits::e1,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        vk::SharingMode::eExclusive,
        0,
        nullptr,
        vk::ImageLayout::eUndefined};
    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    ttlet[atlasImage, atlasImageAllocation] = device.createImage(imageCreateInfo, allocationCreateInfo);

    ttlet clearValue = vk::ClearColorValue{std::array{-1.0f, -1.0f, -1.0f, -1.0f}};
    ttlet clearRange = std::array{vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};

    device.transitionLayout(
        atlasImage, imageCreateInfo.format, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    device.clearColorImage(atlasImage, vk::ImageLayout::eTransferDstOptimal, clearValue, clearRange);

    ttlet atlasImageView = device.createImageView(
        {vk::ImageViewCreateFlags(),
         atlasImage,
         vk::ImageViewType::e2D,
         imageCreateInfo.format,
         vk::ComponentMapping(),
         {
             vk::ImageAspectFlagBits::eColor,
             0, // baseMipLevel
             1, // levelCount
             0, // baseArrayLayer
             1 // layerCount
         }});

    atlasTextures.push_back({atlasImage, atlasImageAllocation, atlasImageView});

    // Build image descriptor info.
    for (int i = 0; i < std::ssize(atlasDescriptorImageInfos); i++) {
        // Point the descriptors to each imageView,
        // repeat the first imageView if there are not enough.
        atlasDescriptorImageInfos.at(i) = {
            vk::Sampler(),
            i < atlasTextures.size() ? atlasTextures.at(i).view : atlasTextures.at(0).view,
            vk::ImageLayout::eShaderReadOnlyOptimal};
    }
}

void device_shared::buildAtlas()
{
    // Create staging image
    vk::ImageCreateInfo const imageCreateInfo = {
        vk::ImageCreateFlags(),
        vk::ImageType::e2D,
        vk::Format::eR8Snorm,
        vk::Extent3D(stagingImageWidth, stagingImageHeight, 1),
        1, // mipLevels
        1, // arrayLayers
        vk::SampleCountFlagBits::e1,
        vk::ImageTiling::eLinear,
        vk::ImageUsageFlagBits::eTransferSrc,
        vk::SharingMode::eExclusive,
        0,
        nullptr,
        vk::ImageLayout::ePreinitialized};
    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    ttlet[image, allocation] = device.createImage(imageCreateInfo, allocationCreateInfo);
    ttlet data = device.mapMemory<sdf_r8>(allocation);

    stagingTexture = {
        image,
        allocation,
        vk::ImageView(),
        tt::pixel_map<sdf_r8>{data.data(), ssize_t{imageCreateInfo.extent.width}, ssize_t{imageCreateInfo.extent.height}}};

    vk::SamplerCreateInfo const samplerCreateInfo = {
        vk::SamplerCreateFlags(),
        vk::Filter::eLinear, // magFilter
        vk::Filter::eLinear, // minFilter
        vk::SamplerMipmapMode::eNearest, // mipmapMode
        vk::SamplerAddressMode::eClampToEdge, // addressModeU
        vk::SamplerAddressMode::eClampToEdge, // addressModeV
        vk::SamplerAddressMode::eClampToEdge, // addressModeW
        0.0, // mipLodBias
        VK_FALSE, // anisotropyEnable
        0.0, // maxAnisotropy
        VK_FALSE, // compareEnable
        vk::CompareOp::eNever,
        0.0, // minLod
        0.0, // maxLod
        vk::BorderColor::eFloatTransparentBlack,
        VK_FALSE // unnormazlizedCoordinates
    };
    atlasSampler = device.createSampler(samplerCreateInfo);

    atlasSamplerDescriptorImageInfo = {atlasSampler, vk::ImageView(), vk::ImageLayout::eUndefined};

    // There needs to be at least one atlas image, so the array of samplers can point to
    // the single image.
    addAtlasImage();
}

void device_shared::teardownAtlas(gui_device_vulkan *vulkanDevice)
{
    tt_axiom(vulkanDevice);

    vulkanDevice->destroy(atlasSampler);

    for (const auto &atlasImage : atlasTextures) {
        vulkanDevice->destroy(atlasImage.view);
        vulkanDevice->destroyImage(atlasImage.image, atlasImage.allocation);
    }
    atlasTextures.clear();

    vulkanDevice->unmapMemory(stagingTexture.allocation);
    vulkanDevice->destroyImage(stagingTexture.image, stagingTexture.allocation);
}

} // namespace tt::pipeline_SDF
