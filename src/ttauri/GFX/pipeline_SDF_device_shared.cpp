// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "pipeline_SDF.hpp"
#include "pipeline_SDF_device_shared.hpp"
#include "gfx_device_vulkan.hpp"
#include "gfx_system.hpp"
#include "../pixel_map.hpp"
#include "../URL.hpp"
#include "../memory.hpp"
#include "../cast.hpp"
#include "../geometry/axis_aligned_rectangle.hpp"
#include "../geometry/scale.hpp"
#include "../geometry/translate.hpp"
#include <array>

namespace tt::inline v1::pipeline_SDF {

device_shared::device_shared(gfx_device_vulkan const &device) : device(device)
{
    buildShaders();
    buildAtlas();
}

device_shared::~device_shared() {}

void device_shared::destroy(gfx_device_vulkan *vulkanDevice)
{
    tt_axiom(vulkanDevice);

    teardownShaders(vulkanDevice);
    teardownAtlas(vulkanDevice);
}

[[nodiscard]] glyph_atlas_info device_shared::allocate_rect(extent2 draw_extent, scale2 draw_scale) noexcept
{
    auto image_width = narrow_cast<int>(std::ceil(draw_extent.width()));
    auto image_height = narrow_cast<int>(std::ceil(draw_extent.height()));

    // Check if the glyph still fits in the same line of glyphs.
    // Otherwise go to the next line.
    if (atlas_allocation_position.x() + image_width > atlasImageWidth) {
        atlas_allocation_position.x() = 0;
        atlas_allocation_position.y() = atlas_allocation_position.y() + atlasAllocationMaxHeight;
        atlasAllocationMaxHeight = 0;
    }

    // Check if the glyph still fits in the image.
    // Otherwise allocate a new image.
    if (atlas_allocation_position.y() + image_height > atlasImageHeight) {
        atlas_allocation_position.x() = 0;
        atlas_allocation_position.y() = 0;
        atlas_allocation_position.z() = atlas_allocation_position.z() + 1;
        atlasAllocationMaxHeight = 0;

        if (atlas_allocation_position.z() >= atlasMaximumNrImages) {
            tt_log_fatal("pipeline_SDF atlas overflow, too many glyphs in use.");
        }

        if (atlas_allocation_position.z() >= atlasTextures.size()) {
            addAtlasImage();
        }
    }

    auto r = glyph_atlas_info{atlas_allocation_position, draw_extent, draw_scale, scale2{atlasTextureCoordinateMultiplier}};
    atlas_allocation_position.x() = atlas_allocation_position.x() + image_width;
    atlasAllocationMaxHeight = std::max(atlasAllocationMaxHeight, image_height);
    return r;
}

void device_shared::uploadStagingPixmapToAtlas(glyph_atlas_info const &location)
{
    // Flush the given image, included the border.
    device.flushAllocation(
        stagingTexture.allocation, 0, (stagingTexture.pixel_map.height() * stagingTexture.pixel_map.stride()) * sizeof(sdf_r8));

    stagingTexture.transitionLayout(device, vk::Format::eR8Snorm, vk::ImageLayout::eTransferSrcOptimal);

    std::array<std::vector<vk::ImageCopy>, atlasMaximumNrImages> regionsToCopyPerAtlasTexture;

    auto regionsToCopy = std::vector{vk::ImageCopy{
        {vk::ImageAspectFlagBits::eColor, 0, 0, 1},
        {0, 0, 0},
        {vk::ImageAspectFlagBits::eColor, 0, 0, 1},
        {narrow_cast<int32_t>(location.position.x()), narrow_cast<int32_t>(location.position.y()), 0},
        {narrow_cast<uint32_t>(location.size.width()), narrow_cast<uint32_t>(location.size.height()), 1}}};

    auto &atlasTexture = atlasTextures.at(narrow_cast<std::size_t>(location.position.z()));
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

void device_shared::prepare_atlas_for_rendering()
{
    ttlet lock = std::scoped_lock(gfx_system_mutex);
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
void device_shared::add_glyph_to_atlas(glyph_ids const &glyph, glyph_atlas_info &info) noexcept
{
    ttlet[glyph_path, glyph_bounding_box] = glyph.get_path_and_bounding_box();

    ttlet draw_scale = scale2{drawfontSize, drawfontSize};
    ttlet draw_bounding_box = draw_scale * glyph_bounding_box;

    // We will draw the font at a fixed size into the texture. And we need a border for the texture to
    // allow proper bi-linear interpolation on the edges.

    // Determine the size of the image in the atlas.
    // This is the bounding box sized to the fixed font size and a border
    ttlet draw_offset = point2{drawBorder, drawBorder} - get<0>(draw_bounding_box);
    ttlet draw_extent = draw_bounding_box.size() + 2.0f * drawBorder;
    ttlet image_size = ceil(draw_extent);

    // Transform the path to the scale of the fixed font size and drawing the bounding box inside the image.
    ttlet draw_path = (translate2{draw_offset} * draw_scale) * glyph_path;

    // Draw glyphs into staging buffer of the atlas and upload it to the correct position in the atlas.
    ttlet lock = std::scoped_lock(gfx_system_mutex);
    prepareStagingPixmapForDrawing();
    info = allocate_rect(image_size, image_size / draw_bounding_box.size());
    auto pixmap = stagingTexture.pixel_map.submap(aarectangle{info.size});
    fill(pixmap, draw_path);
    uploadStagingPixmapToAtlas(info);
}

aarectangle device_shared::get_bounding_box(glyph_ids const &glyphs) const noexcept
{
    // Adjust bounding box by adding a border based on 1EM.
    return glyphs.get_bounding_box() + scaledDrawBorder;
}

bool device_shared::place_vertices(
    vspan<vertex> &vertices,
    aarectangle const &clipping_rectangle,
    quad const &box,
    glyph_ids const &glyphs,
    quad_color colors) noexcept
{
    ttlet[atlas_rect, glyph_was_added] = get_glyph_from_atlas(glyphs);

    ttlet box_with_border = scale_from_center(box, atlas_rect->border_scale);

    auto image_index = atlas_rect->position.z();
    auto t0 = point3(get<0>(atlas_rect->texture_coordinates), image_index);
    auto t1 = point3(get<1>(atlas_rect->texture_coordinates), image_index);
    auto t2 = point3(get<2>(atlas_rect->texture_coordinates), image_index);
    auto t3 = point3(get<3>(atlas_rect->texture_coordinates), image_index);


    vertices.emplace_back(box_with_border.p0, clipping_rectangle, t0, colors.p0);
    vertices.emplace_back(box_with_border.p1, clipping_rectangle, t1, colors.p1);
    vertices.emplace_back(box_with_border.p2, clipping_rectangle, t2, colors.p2);
    vertices.emplace_back(box_with_border.p3, clipping_rectangle, t3, colors.p3);
    return glyph_was_added;
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

void device_shared::teardownShaders(gfx_device_vulkan *vulkanDevice)
{
    tt_axiom(vulkanDevice);

    vulkanDevice->destroy(vertexShaderModule);
    vulkanDevice->destroy(fragmentShaderModule);
}

void device_shared::addAtlasImage()
{
    ttlet current_image_index = atlasTextures.size();

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
    auto allocation_name = std::format("sdf-pipeline atlas image {}", current_image_index);
    allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
    allocationCreateInfo.pUserData = const_cast<char *>(allocation_name.c_str());
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    ttlet[atlasImage, atlasImageAllocation] = device.createImage(imageCreateInfo, allocationCreateInfo);
    device.setDebugUtilsObjectNameEXT(atlasImage, allocation_name.c_str());

    ttlet clearValue = vk::ClearColorValue{std::array{-1.0f, -1.0f, -1.0f, -1.0f}};
    ttlet clearRange = std::array{vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};

    device.transition_layout(
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
    for (int i = 0; i < ssize(atlasDescriptorImageInfos); i++) {
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
    allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
    allocationCreateInfo.pUserData = const_cast<char *>("sdf-pipeline staging image");
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    ttlet[image, allocation] = device.createImage(imageCreateInfo, allocationCreateInfo);
    device.setDebugUtilsObjectNameEXT(image, "sdf-pipeline staging image");
    ttlet data = device.mapMemory<sdf_r8>(allocation);

    stagingTexture = {
        image,
        allocation,
        vk::ImageView(),
        tt::pixel_map<sdf_r8>{data.data(), imageCreateInfo.extent.width, imageCreateInfo.extent.height}};

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
    device.setDebugUtilsObjectNameEXT(atlasSampler, "sdf-pipeline atlas sampler");

    atlasSamplerDescriptorImageInfo = {atlasSampler, vk::ImageView(), vk::ImageLayout::eUndefined};

    // There needs to be at least one atlas image, so the array of samplers can point to
    // the single image.
    addAtlasImage();
}

void device_shared::teardownAtlas(gfx_device_vulkan *vulkanDevice)
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

} // namespace tt::inline v1::pipeline_SDF
