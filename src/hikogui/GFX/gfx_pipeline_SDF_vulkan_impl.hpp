// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "gfx_pipeline_SDF_vulkan_intf.hpp"
#include "gfx_surface_vulkan_intf.hpp"
#include "gfx_device_vulkan_impl.hpp"
#include "draw_context_intf.hpp"
#include "../macros.hpp"
#include <vulkan/vulkan.hpp>

hi_export_module(hikogui.GFX : gfx_pipeline_SDF_impl);

hi_export namespace hi { inline namespace v1 {

hi_inline void gfx_pipeline_SDF::draw_in_command_buffer(vk::CommandBuffer commandBuffer, draw_context const& context)
{
    gfx_pipeline::draw_in_command_buffer(commandBuffer, context);

    hi_axiom_not_null(device());
    device()->flushAllocation(vertexBufferAllocation, 0, vertexBufferData.size() * sizeof(vertex));

    std::vector<vk::Buffer> tmpvertexBuffers = {vertexBuffer};
    std::vector<vk::DeviceSize> tmpOffsets = {0};
    hi_assert(tmpvertexBuffers.size() == tmpOffsets.size());

    device()->SDF_pipeline->drawInCommandBuffer(commandBuffer);

    commandBuffer.bindVertexBuffers(0, tmpvertexBuffers, tmpOffsets);

    pushConstants.window_extent = extent2{narrow_cast<float>(extent.width), narrow_cast<float>(extent.height)};
    pushConstants.viewport_scale = scale2{narrow_cast<float>(2.0f / extent.width), narrow_cast<float>(2.0f / extent.height)};
    pushConstants.has_subpixels = context.subpixel_orientation != subpixel_orientation::unknown;

    constexpr float third = 1.0f / 3.0f;
    switch (context.subpixel_orientation) {
    case subpixel_orientation::unknown:
        pushConstants.red_subpixel_offset = vector2{0.0f, 0.0f};
        pushConstants.blue_subpixel_offset = vector2{0.0f, 0.0f};
        break;
    case subpixel_orientation::horizontal_rgb:
        pushConstants.red_subpixel_offset = vector2{-third, 0.0f};
        pushConstants.blue_subpixel_offset = vector2{third, 0.0f};
        break;
    case subpixel_orientation::horizontal_bgr:
        pushConstants.red_subpixel_offset = vector2{third, 0.0f};
        pushConstants.blue_subpixel_offset = vector2{-third, 0.0f};
        break;
    case subpixel_orientation::vertical_rgb:
        pushConstants.red_subpixel_offset = vector2{0.0f, third};
        pushConstants.blue_subpixel_offset = vector2{0.0f, -third};
        break;
    case subpixel_orientation::vertical_bgr:
        pushConstants.red_subpixel_offset = vector2{0.0f, -third};
        pushConstants.blue_subpixel_offset = vector2{0.0f, third};
        break;
    default:
        hi_no_default();
    }

    commandBuffer.pushConstants(
        pipelineLayout,
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
        0,
        sizeof(push_constants),
        &pushConstants);

    auto const numberOfRectangles = vertexBufferData.size() / 4;
    auto const numberOfTriangles = numberOfRectangles * 2;
    device()->cmdBeginDebugUtilsLabelEXT(commandBuffer, "draw glyphs");
    commandBuffer.drawIndexed(narrow_cast<uint32_t>(numberOfTriangles * 3), 1, 0, 0, 0);
    device()->cmdEndDebugUtilsLabelEXT(commandBuffer);
}

hi_inline std::vector<vk::PipelineShaderStageCreateInfo> gfx_pipeline_SDF::createShaderStages() const
{
    hi_axiom_not_null(device());
    return device()->SDF_pipeline->shaderStages;
}

/* Dual-source alpha blending which allows subpixel anti-aliasing.
 */
hi_inline std::vector<vk::PipelineColorBlendAttachmentState> gfx_pipeline_SDF::getPipelineColorBlendAttachmentStates() const
{
    bool has_dual_source_blend = false;
    if (auto device_ = device()) {
        has_dual_source_blend = device_->device_features.dualSrcBlend;
    }

    return {
        {VK_TRUE, // blendEnable
         vk::BlendFactor::eOne, // srcColorBlendFactor
         has_dual_source_blend ? vk::BlendFactor::eOneMinusSrc1Color : vk::BlendFactor::eOneMinusSrcAlpha, // dstColorBlendFactor
         vk::BlendOp::eAdd, // colorBlendOp
         vk::BlendFactor::eOne, // srcAlphaBlendFactor
         has_dual_source_blend ? vk::BlendFactor::eOneMinusSrc1Alpha : vk::BlendFactor::eOneMinusSrcAlpha, // dstAlphaBlendFactor
         vk::BlendOp::eAdd, // aphaBlendOp
         vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
             vk::ColorComponentFlagBits::eA}};
}

hi_inline std::vector<vk::DescriptorSetLayoutBinding> gfx_pipeline_SDF::createDescriptorSetLayoutBindings() const
{
    return {
        {0, // binding
         vk::DescriptorType::eSampler,
         1, // descriptorCount
         vk::ShaderStageFlagBits::eFragment},
        {1, // binding
         vk::DescriptorType::eSampledImage,
         narrow_cast<uint32_t>(device_shared::atlasMaximumNrImages), // descriptorCount
         vk::ShaderStageFlagBits::eFragment}};
}

hi_inline std::vector<vk::WriteDescriptorSet> gfx_pipeline_SDF::createWriteDescriptorSet() const
{
    hi_axiom_not_null(device());
    auto const& sharedImagePipeline = device()->SDF_pipeline;

    return {
        {
            descriptorSet,
            0, // destBinding
            0, // arrayElement
            1, // descriptorCount
            vk::DescriptorType::eSampler,
            &sharedImagePipeline->atlasSamplerDescriptorImageInfo,
            nullptr, // bufferInfo
            nullptr // texelBufferView
        },
        {
            descriptorSet,
            1, // destBinding
            0, // arrayElement
            narrow_cast<uint32_t>(device_shared::atlasMaximumNrImages), // descriptorCount
            vk::DescriptorType::eSampledImage,
            sharedImagePipeline->atlasDescriptorImageInfos.data(),
            nullptr, // bufferInfo
            nullptr // texelBufferView
        },
    };
}

hi_inline size_t gfx_pipeline_SDF::getDescriptorSetVersion() const
{
    hi_axiom_not_null(device());
    return device()->SDF_pipeline->atlasTextures.size();
}

hi_inline std::vector<vk::PushConstantRange> gfx_pipeline_SDF::createPushConstantRanges() const
{
    return push_constants::pushConstantRanges();
}

hi_inline vk::VertexInputBindingDescription gfx_pipeline_SDF::createVertexInputBindingDescription() const
{
    return vertex::inputBindingDescription();
}

hi_inline std::vector<vk::VertexInputAttributeDescription> gfx_pipeline_SDF::createVertexInputAttributeDescriptions() const
{
    return vertex::inputAttributeDescriptions();
}

hi_inline void gfx_pipeline_SDF::build_vertex_buffers()
{
    using vertexIndexType = uint16_t;
    constexpr ssize_t numberOfVertices = 1 << (sizeof(vertexIndexType) * CHAR_BIT);

    vk::BufferCreateInfo const bufferCreateInfo = {
        vk::BufferCreateFlags(),
        sizeof(vertex) * numberOfVertices,
        vk::BufferUsageFlagBits::eVertexBuffer,
        vk::SharingMode::eExclusive};
    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
    allocationCreateInfo.pUserData = const_cast<char *>("sdf-pipeline vertex buffer");
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    hi_axiom_not_null(device());
    std::tie(vertexBuffer, vertexBufferAllocation) = device()->createBuffer(bufferCreateInfo, allocationCreateInfo);
    device()->setDebugUtilsObjectNameEXT(vertexBuffer, "sdf-pipeline vertex buffer");
    vertexBufferData = device()->mapMemory<vertex>(vertexBufferAllocation);
}

hi_inline void gfx_pipeline_SDF::teardown_vertex_buffers()
{
    hi_axiom_not_null(device());
    device()->unmapMemory(vertexBufferAllocation);
    device()->destroyBuffer(vertexBuffer, vertexBufferAllocation);
}

hi_inline void gfx_pipeline_SDF::texture_map::transitionLayout(const gfx_device &device, vk::Format format, vk::ImageLayout nextLayout)
{
    hi_axiom(gfx_system_mutex.recurse_lock_count());

    if (layout != nextLayout) {
        device.transition_layout(image, format, layout, nextLayout);
        layout = nextLayout;
    }
}

hi_inline gfx_pipeline_SDF::device_shared::device_shared(gfx_device const& device) : device(device)
{
    buildShaders();
    buildAtlas();
}

hi_inline gfx_pipeline_SDF::device_shared::~device_shared() {}

hi_inline void gfx_pipeline_SDF::device_shared::destroy(gfx_device const *vulkanDevice)
{
    hi_assert_not_null(vulkanDevice);

    teardownShaders(vulkanDevice);
    teardownAtlas(vulkanDevice);
}

[[nodiscard]] hi_inline glyph_atlas_info gfx_pipeline_SDF::device_shared::allocate_rect(extent2 draw_extent, scale2 draw_scale) noexcept
{
    auto image_width = ceil_cast<int>(draw_extent.width());
    auto image_height = ceil_cast<int>(draw_extent.height());

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
            hi_log_fatal("gfx_pipeline_SDF atlas overflow, too many glyphs in use.");
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

hi_inline void gfx_pipeline_SDF::device_shared::uploadStagingPixmapToAtlas(glyph_atlas_info const& location)
{
    // Flush the given image, included the border.
    device.flushAllocation(
        stagingTexture.allocation, 0, (stagingTexture.pixmap.height() * stagingTexture.pixmap.stride()) * sizeof(sdf_r8));

    stagingTexture.transitionLayout(device, vk::Format::eR8Snorm, vk::ImageLayout::eTransferSrcOptimal);

    std::array<std::vector<vk::ImageCopy>, atlasMaximumNrImages> regionsToCopyPerAtlasTexture;

    auto regionsToCopy = std::vector{vk::ImageCopy{
        {vk::ImageAspectFlagBits::eColor, 0, 0, 1},
        {0, 0, 0},
        {vk::ImageAspectFlagBits::eColor, 0, 0, 1},
        {floor_cast<int32_t>(location.position.x()), floor_cast<int32_t>(location.position.y()), 0},
        {ceil_cast<uint32_t>(location.size.width()), ceil_cast<uint32_t>(location.size.height()), 1}}};

    auto& atlasTexture = atlasTextures.at(floor_cast<std::size_t>(location.position.z()));
    atlasTexture.transitionLayout(device, vk::Format::eR8Snorm, vk::ImageLayout::eTransferDstOptimal);

    device.copyImage(
        stagingTexture.image,
        vk::ImageLayout::eTransferSrcOptimal,
        atlasTexture.image,
        vk::ImageLayout::eTransferDstOptimal,
        std::move(regionsToCopy));
}

hi_inline void gfx_pipeline_SDF::device_shared::prepareStagingPixmapForDrawing()
{
    stagingTexture.transitionLayout(device, vk::Format::eR8Snorm, vk::ImageLayout::eGeneral);
}

hi_inline void gfx_pipeline_SDF::device_shared::prepare_atlas_for_rendering()
{
    auto const lock = std::scoped_lock(gfx_system_mutex);
    for (auto& atlasTexture : atlasTextures) {
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
hi_inline void gfx_pipeline_SDF::device_shared::add_glyph_to_atlas(hi::font const &font, glyph_id glyph, glyph_atlas_info& info) noexcept
{
    auto const glyph_metrics = font.get_metrics(glyph);
    auto const glyph_path = font.get_path(glyph);
    auto const glyph_bounding_box = glyph_metrics.bounding_rectangle;

    auto const draw_scale = scale2{drawfontSize, drawfontSize};
    auto const draw_bounding_box = draw_scale * glyph_bounding_box;

    // We will draw the font at a fixed size into the texture. And we need a border for the texture to
    // allow proper bi-linear interpolation on the edges.

    // Determine the size of the image in the atlas.
    // This is the bounding box sized to the fixed font size and a border
    auto const draw_offset = point2{drawBorder, drawBorder} - get<0>(draw_bounding_box);
    auto const draw_extent = draw_bounding_box.size() + 2.0f * drawBorder;
    auto const image_size = ceil(draw_extent);

    // Transform the path to the scale of the fixed font size and drawing the bounding box inside the image.
    auto const draw_path = (translate2{draw_offset} * draw_scale) * glyph_path;

    // Draw glyphs into staging buffer of the atlas and upload it to the correct position in the atlas.
    auto const lock = std::scoped_lock(gfx_system_mutex);
    prepareStagingPixmapForDrawing();
    info = allocate_rect(image_size, image_size / draw_bounding_box.size());
    auto pixmap =
        stagingTexture.pixmap.subimage(0, 0, ceil_cast<size_t>(info.size.width()), ceil_cast<size_t>(info.size.height()));
    fill(pixmap, draw_path);
    uploadStagingPixmapToAtlas(info);
}

hi_inline bool gfx_pipeline_SDF::device_shared::place_vertices(
    vector_span<vertex>& vertices,
    aarectangle const& clipping_rectangle,
    quad const& box,
    hi::font const &font, glyph_id glyph,
    quad_color colors) noexcept
{
    auto const[atlas_rect, glyph_was_added] = this->get_glyph_from_atlas(font, glyph);

    auto const box_with_border = scale_from_center(box, atlas_rect->border_scale);

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

hi_inline void gfx_pipeline_SDF::device_shared::drawInCommandBuffer(vk::CommandBuffer const& commandBuffer)
{
    commandBuffer.bindIndexBuffer(device.quadIndexBuffer, 0, vk::IndexType::eUint16);
}

hi_inline void gfx_pipeline_SDF::device_shared::buildShaders()
{
    specializationConstants.sdf_r8maxDistance = sdf_r8::max_distance;
    specializationConstants.atlasImageWidth = atlasImageWidth;

    fragmentShaderSpecializationMapEntries = specialization_constants::specializationConstantMapEntries();
    fragmentShaderSpecializationInfo = specializationConstants.specializationInfo(fragmentShaderSpecializationMapEntries);

    vertexShaderModule = device.loadShader(URL("resource:SDF_vulkan.vert.spv"));
    fragmentShaderModule = device.loadShader(URL("resource:SDF_vulkan.frag.spv"));

    shaderStages = {
        {vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main"},
        {vk::PipelineShaderStageCreateFlags(),
         vk::ShaderStageFlagBits::eFragment,
         fragmentShaderModule,
         "main",
         &fragmentShaderSpecializationInfo}};
}

hi_inline void gfx_pipeline_SDF::device_shared::teardownShaders(gfx_device const *vulkanDevice)
{
    hi_assert_not_null(vulkanDevice);

    vulkanDevice->destroy(vertexShaderModule);
    vulkanDevice->destroy(fragmentShaderModule);
}

hi_inline void gfx_pipeline_SDF::device_shared::addAtlasImage()
{
    auto const current_image_index = atlasTextures.size();

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

    auto const[atlasImage, atlasImageAllocation] = device.createImage(imageCreateInfo, allocationCreateInfo);
    device.setDebugUtilsObjectNameEXT(atlasImage, allocation_name.c_str());

    auto const clearValue = vk::ClearColorValue{std::array{-1.0f, -1.0f, -1.0f, -1.0f}};
    auto const clearRange = std::array{vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};

    device.transition_layout(
        atlasImage, imageCreateInfo.format, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    device.clearColorImage(atlasImage, vk::ImageLayout::eTransferDstOptimal, clearValue, clearRange);

    auto const atlasImageView = device.createImageView(
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

hi_inline void gfx_pipeline_SDF::device_shared::buildAtlas()
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
    auto const[image, allocation] = device.createImage(imageCreateInfo, allocationCreateInfo);
    device.setDebugUtilsObjectNameEXT(image, "sdf-pipeline staging image");
    auto const data = device.mapMemory<sdf_r8>(allocation);

    stagingTexture = {
        image,
        allocation,
        vk::ImageView(),
        hi::pixmap_span<sdf_r8>{data.data(), imageCreateInfo.extent.width, imageCreateInfo.extent.height}};

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

hi_inline void gfx_pipeline_SDF::device_shared::teardownAtlas(gfx_device const *vulkanDevice)
{
    hi_assert_not_null(vulkanDevice);

    vulkanDevice->destroy(atlasSampler);

    for (const auto& atlasImage : atlasTextures) {
        vulkanDevice->destroy(atlasImage.view);
        vulkanDevice->destroyImage(atlasImage.image, atlasImage.allocation);
    }
    atlasTextures.clear();

    vulkanDevice->unmapMemory(stagingTexture.allocation);
    vulkanDevice->destroyImage(stagingTexture.image, stagingTexture.allocation);
}

}} // namespace hi::inline v1::gfx_pipeline_SDF
