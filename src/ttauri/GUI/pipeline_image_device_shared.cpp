// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "pipeline_image.hpp"
#include "pipeline_image_device_shared.hpp"
#include "pipeline_image_image.hpp"
#include "gui_device_vulkan.hpp"
#include "../pixel_map.hpp"
#include "../pixel_map.inl"
#include "../URL.hpp"
#include "../memory.hpp"
#include "../cast.hpp"
#include <array>

namespace tt::pipeline_image {

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

std::vector<Page> device_shared::allocatePages(size_t num_pages) noexcept
{
    while (num_pages > atlasFreePages.size()) {
        addAtlasImage();
    }

    auto pages = std::vector<Page>();
    for (int i = 0; i < num_pages; i++) {
        ttlet page = atlasFreePages.back();
        pages.push_back(page);
        atlasFreePages.pop_back();
    }
    return pages;
}

void device_shared::freePages(std::vector<Page> const &pages) noexcept
{
    atlasFreePages.insert(atlasFreePages.end(), pages.begin(), pages.end());
}

Image device_shared::makeImage(size_t width, size_t height) noexcept
{
    ttlet width_in_pages = (width + (Page::width - 1)) / Page::width;
    ttlet height_in_pages = (height + (Page::height - 1)) / Page::height;
    ttlet nr_pages = width_in_pages * height_in_pages;

    return Image{this, width, height, width_in_pages, height_in_pages, allocatePages(nr_pages)};
}

tt::pixel_map<sfloat_rgba16> device_shared::getStagingPixelMap()
{
    stagingTexture.transitionLayout(device, vk::Format::eR16G16B16A16Sfloat, vk::ImageLayout::eGeneral);

    return stagingTexture.pixel_map.submap(
        Page::border, Page::border, stagingImageWidth - 2 * Page::border, stagingImageHeight - 2 * Page::border);
}

void device_shared::updateAtlasWithStagingPixelMap(const Image &image)
{
    // Start with the actual image inside the stagingImage.
    auto rectangle = aarectangle{
        point2{narrow_cast<float>(Page::border), narrow_cast<float>(Page::border)},
        extent2{narrow_cast<float>(image.width_in_px), narrow_cast<float>(image.height_in_px)}};

    // Add one pixel of border around the actual image and keep extending
    // until the full border width is finished.
    for (int b = 0; b < Page::border; b++) {
        rectangle = expand(rectangle, 1);

        auto pixel_map = stagingTexture.pixel_map.submap(rectangle);
        makeTransparentBorder(pixel_map);
    }

    // Flush the given image, included the border.
    device.flushAllocation(
        stagingTexture.allocation,
        0,
        ((image.width_in_px + 2 * Page::border) * stagingTexture.pixel_map.stride()) * sizeof(uint32_t));

    stagingTexture.transitionLayout(device, vk::Format::eR16G16B16A16Sfloat, vk::ImageLayout::eTransferSrcOptimal);

    array<vector<vk::ImageCopy>, atlasMaximumNrImages> regionsToCopyPerAtlasTexture;
    for (int index = 0; index < std::ssize(image.pages); index++) {
        ttlet page = image.pages.at(index);

        if (page.isFullyTransparent()) {
            // Hole in the image does not need to be rendered.
            continue;
        }

        ttlet add_border_translate = translate2{narrow_cast<float>(Page::border), narrow_cast<float>(Page::border)};
        ttlet remove_border_translate = ~add_border_translate;

        ttlet imageRect = image.index_to_rect(index);
        // Adjust the position to be inside the stagingImage, excluding its border.
        ttlet imageRectInStagingImage = add_border_translate * imageRect;

        // During copying we want to copy extra pixels around each page, this allows for non-nearest-neighbor sampling
        // on the edge of a page.
        ttlet imageRectToCopy = expand(imageRectInStagingImage, narrow_cast<float>(Page::border));

        // We are copying the border into the atlas as well.
        ttlet atlasPositionIncludingBorder = remove_border_translate * getAtlasPositionFromPage(page);

        auto &regionsToCopy = regionsToCopyPerAtlasTexture.at(narrow_cast<size_t>(atlasPositionIncludingBorder.z()));
        regionsToCopy.push_back(
            {{vk::ImageAspectFlagBits::eColor, 0, 0, 1},
             {narrow_cast<int32_t>(imageRectToCopy.left()), narrow_cast<int32_t>(imageRectToCopy.bottom()), 0},
             {vk::ImageAspectFlagBits::eColor, 0, 0, 1},
             {narrow_cast<int32_t>(atlasPositionIncludingBorder.x()), narrow_cast<int32_t>(atlasPositionIncludingBorder.y()), 0},
             {narrow_cast<uint32_t>(imageRectToCopy.width()), narrow_cast<uint32_t>(imageRectToCopy.height()), 1}});
    }

    for (int atlasTextureIndex = 0; atlasTextureIndex < std::ssize(atlasTextures); atlasTextureIndex++) {
        ttlet &regionsToCopy = regionsToCopyPerAtlasTexture.at(atlasTextureIndex);
        if (regionsToCopy.size() == 0) {
            continue;
        }

        auto &atlasTexture = atlasTextures.at(atlasTextureIndex);
        atlasTexture.transitionLayout(device, vk::Format::eR16G16B16A16Sfloat, vk::ImageLayout::eTransferDstOptimal);

        device.copyImage(
            stagingTexture.image,
            vk::ImageLayout::eTransferSrcOptimal,
            atlasTexture.image,
            vk::ImageLayout::eTransferDstOptimal,
            regionsToCopy);
    }
}

void device_shared::prepareAtlasForRendering()
{
    for (auto &atlasTexture : atlasTextures) {
        atlasTexture.transitionLayout(device, vk::Format::eR16G16B16A16Sfloat, vk::ImageLayout::eShaderReadOnlyOptimal);
    }
}

void device_shared::drawInCommandBuffer(vk::CommandBuffer &commandBuffer)
{
    commandBuffer.bindIndexBuffer(device.quadIndexBuffer, 0, vk::IndexType::eUint16);
}

void device_shared::buildShaders()
{
    vertexShaderModule = device.loadShader(URL("resource:GUI/pipeline_image.vert.spv"));
    fragmentShaderModule = device.loadShader(URL("resource:GUI/pipeline_image.frag.spv"));

    shaderStages = {
        {vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main"},
        {vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment, fragmentShaderModule, "main"}};
}

void device_shared::teardownShaders(gui_device_vulkan *vulkanDevice)
{
    tt_axiom(vulkanDevice);
    vulkanDevice->destroy(vertexShaderModule);
    vulkanDevice->destroy(fragmentShaderModule);
}

void device_shared::addAtlasImage()
{
    ttlet currentImageIndex = std::ssize(atlasTextures);

    // Create atlas image
    vk::ImageCreateInfo const imageCreateInfo = {
        vk::ImageCreateFlags(),
        vk::ImageType::e2D,
        vk::Format::eR16G16B16A16Sfloat,
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

    // Add pages for this image to free list.
    ttlet pageOffset = currentImageIndex * atlasNrPagesPerImage;
    for (int i = 0; i < atlasNrPagesPerImage; i++) {
        atlasFreePages.push_back({pageOffset + i});
    }

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
        vk::Format::eR16G16B16A16Sfloat,
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
    ttlet data = device.mapMemory<sfloat_rgba16>(allocation);

    stagingTexture = {
        image,
        allocation,
        vk::ImageView(),
        tt::pixel_map<sfloat_rgba16>{data.data(), imageCreateInfo.extent.width, imageCreateInfo.extent.height}};

    vk::SamplerCreateInfo const samplerCreateInfo = {
        vk::SamplerCreateFlags(),
        vk::Filter::eLinear, // magFilter
        vk::Filter::eLinear, // minFilter
        vk::SamplerMipmapMode::eNearest, // mipmapMode
        vk::SamplerAddressMode::eRepeat, // addressModeU
        vk::SamplerAddressMode::eRepeat, // addressModeV
        vk::SamplerAddressMode::eRepeat, // addressModeW
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

} // namespace tt::pipeline_image
