// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/GUI/PipelineImage.hpp"
#include "TTauri/GUI/PipelineImage_DeviceShared.hpp"
#include "TTauri/GUI/PipelineImage_Image.hpp"
#include "TTauri/GUI/Device.hpp"
#include "TTauri/Foundation/PixelMap.hpp"
#include "TTauri/Foundation/URL.hpp"
#include "TTauri/Foundation/memory.hpp"
#include "TTauri/Foundation/numeric_cast.hpp"
#include <array>

namespace TTauri::GUI::PipelineImage {

using namespace std;

DeviceShared::DeviceShared(Device const &device) :
    device(device)
{
    buildShaders();
    buildAtlas();
}

DeviceShared::~DeviceShared()
{
}

void DeviceShared::destroy(gsl::not_null<Device *> vulkanDevice)
{
    teardownShaders(vulkanDevice);
    teardownAtlas(vulkanDevice);
}

std::vector<Page> DeviceShared::getFreePages(int const nrPages)
{
    while (nrPages > atlasFreePages.size()) {
        addAtlasImage();
    }

    auto pages = std::vector<Page>();
    for (int i = 0; i < nrPages; i++) {
        let page = atlasFreePages.back();
        pages.push_back(page);
        atlasFreePages.pop_back();
    }
    return pages;
}

void DeviceShared::returnPages(std::vector<Page> const &pages)
{

    atlasFreePages.insert(atlasFreePages.end(), pages.begin(), pages.end());
}

std::shared_ptr<Image> DeviceShared::getImage(std::string const &key, const ivec extent)
{

    let i = imageCache.find(key);
    if (i != imageCache.end()) {
        if (let image = i->second.lock()) {
            return image;
        }
    }

    // Cleanup only after the happy flow failed.
    cleanupWeakPointers(imageCache);

    let pageExtent = ivec{
        (extent.x() + (Page::width - 1)) / Page::width,
        (extent.y() + (Page::height - 1)) / Page::height
    };

    let pages = getFreePages(pageExtent.x() * pageExtent.y());
    let image = std::make_shared<Image>(this, key, extent, pageExtent, pages);

    imageCache.try_emplace(key, image);
    return image;
}

TTauri::PixelMap<uint32_t> DeviceShared::getStagingPixelMap()
{

    stagingTexture.transitionLayout(device, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eGeneral);

    return stagingTexture.pixelMap.submap(
        Page::border, Page::border,
        stagingImageWidth - 2 * Page::border, stagingImageHeight - 2 * Page::border
    );
}

void DeviceShared::updateAtlasWithStagingPixelMap(const Image &image)
{
    // Start with the actual image inside the stagingImage.
    auto rectangle = irect{
        ivec{Page::border, Page::border},
        image.extent
    };
    // Add one pixel of border around the actual image and keep extending
    // until the full border with is finished.
    for (int b = 0; b < Page::border; b++) {
        rectangle = expand(rectangle, 1);

        auto pixelMap = stagingTexture.pixelMap.submap(rectangle);
        addTransparentBorder(pixelMap);
    }

    // Flush the given image, included the border.
    device.flushAllocation(
        stagingTexture.allocation,
        0,
        ((image.extent.x() + 2 * Page::border) * stagingTexture.pixelMap.stride) * sizeof (uint32_t)
    );
    
    stagingTexture.transitionLayout(device, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eTransferSrcOptimal);

    array<vector<vk::ImageCopy>, atlasMaximumNrImages> regionsToCopyPerAtlasTexture; 
    for (int index = 0; index < to_signed(image.pages.size()); index++) {
        let page = image.pages.at(index);

        if (page.isFullyTransparent()) {
            // Hole in the image does not need to be rendered.
            continue;
        }

        let imageRect = image.indexToRect(index);
        // Adjust the position to be inside the stagingImage, excluding its border.
        let imageRectInStagingImage = imageRect + ivec(Page::border, Page::border);

        // During copying we want to copy extra pixels around each page, this allows for non-nearest-neighbor sampling
        // on the edge of a page.
        let imageRectToCopy = expand(imageRectInStagingImage, Page::border);

        // We are copying the border into the atlas as well.
        let atlasPositionIncludingBorder = getAtlasPositionFromPage(page) - ivec(Page::border, Page::border);

        auto &regionsToCopy = regionsToCopyPerAtlasTexture.at(atlasPositionIncludingBorder.z());
        regionsToCopy.push_back({
            { vk::ImageAspectFlagBits::eColor, 0, 0, 1 },
            { numeric_cast<int32_t>(imageRectToCopy.x1()), numeric_cast<int32_t>(imageRectToCopy.y1()), 0 },
            { vk::ImageAspectFlagBits::eColor, 0, 0, 1 },
            { numeric_cast<int32_t>(atlasPositionIncludingBorder.x()), numeric_cast<int32_t>(atlasPositionIncludingBorder.y()), 0 },
            { numeric_cast<uint32_t>(imageRectToCopy.width()), numeric_cast<uint32_t>(imageRectToCopy.height()), 1}
        });
    }

    for (int atlasTextureIndex = 0; atlasTextureIndex < to_signed(atlasTextures.size()); atlasTextureIndex++) {
        let &regionsToCopy = regionsToCopyPerAtlasTexture.at(atlasTextureIndex);
        if (regionsToCopy.size() == 0) {
            continue;
        }

        auto &atlasTexture = atlasTextures.at(atlasTextureIndex);
        atlasTexture.transitionLayout(device, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eTransferDstOptimal);

        device.copyImage(stagingTexture.image, vk::ImageLayout::eTransferSrcOptimal, atlasTexture.image, vk::ImageLayout::eTransferDstOptimal, regionsToCopy);
    }
}

void DeviceShared::uploadPixmapToAtlas(Image const &image, PixelMap<wsRGBA> const &pixelMap)
{
    if (image.state == GUI::PipelineImage::Image::State::Drawing && pixelMap) {
        auto stagingMap = getStagingPixelMap(image.extent);
        fill(stagingMap, pixelMap);
        updateAtlasWithStagingPixelMap(image);
        image.state = GUI::PipelineImage::Image::State::Uploaded;
    }
}

void DeviceShared::prepareAtlasForRendering()
{
    for (auto &atlasTexture: atlasTextures) {
        atlasTexture.transitionLayout(device, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eShaderReadOnlyOptimal);
    }
}

void DeviceShared::drawInCommandBuffer(vk::CommandBuffer &commandBuffer)
{
    commandBuffer.bindIndexBuffer(device.quadIndexBuffer, 0, vk::IndexType::eUint16);
}

void DeviceShared::buildShaders()
{
    vertexShaderModule = device.loadShader(URL("resource:GUI/PipelineImage.vert.spv"));
    fragmentShaderModule = device.loadShader(URL("resource:GUI/PipelineImage.frag.spv"));

    shaderStages = {
        {vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main"},
        {vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment, fragmentShaderModule, "main"}
    };
}

void DeviceShared::teardownShaders(gsl::not_null<Device_vulkan *> vulkanDevice)
{
    vulkanDevice->destroy(vertexShaderModule);
    vulkanDevice->destroy(fragmentShaderModule);
}

void DeviceShared::addAtlasImage()
{
    let currentImageIndex = to_signed(atlasTextures.size());

    // Create atlas image
    vk::ImageCreateInfo const imageCreateInfo = {
        vk::ImageCreateFlags(),
        vk::ImageType::e2D,
        vk::Format::eR8G8B8A8Srgb,
        vk::Extent3D(atlasImageWidth, atlasImageHeight, 1),
        1, // mipLevels
        1, // arrayLayers
        vk::SampleCountFlagBits::e1,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        vk::SharingMode::eExclusive,
        0, nullptr,
        vk::ImageLayout::eUndefined
    };
    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    let [atlasImage, atlasImageAllocation] = device.createImage(imageCreateInfo, allocationCreateInfo);

    let atlasImageView = device.createImageView({
        vk::ImageViewCreateFlags(),
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
        }
    });

    atlasTextures.push_back({ atlasImage, atlasImageAllocation, atlasImageView });
 
    // Add pages for this image to free list.
    let pageOffset = currentImageIndex * atlasNrPagesPerImage;
    for (int i = 0; i < atlasNrPagesPerImage; i++) {
            atlasFreePages.push_back({pageOffset + i});
    }

    // Build image descriptor info.
    for (int i = 0; i < to_signed(atlasDescriptorImageInfos.size()); i++) {
        // Point the descriptors to each imageView,
        // repeat the first imageView if there are not enough.
        atlasDescriptorImageInfos.at(i) = {
            vk::Sampler(),
            i < atlasTextures.size() ? atlasTextures.at(i).view : atlasTextures.at(0).view,
            vk::ImageLayout::eShaderReadOnlyOptimal
        };
    }
}

void DeviceShared::buildAtlas()
{
    // Create staging image
    vk::ImageCreateInfo const imageCreateInfo = {
        vk::ImageCreateFlags(),
        vk::ImageType::e2D,
        vk::Format::eR8G8B8A8Srgb,
        vk::Extent3D(stagingImageWidth, stagingImageHeight, 1),
        1, // mipLevels
        1, // arrayLayers
        vk::SampleCountFlagBits::e1,
        vk::ImageTiling::eLinear,
        vk::ImageUsageFlagBits::eTransferSrc,
        vk::SharingMode::eExclusive,
        0, nullptr,
        vk::ImageLayout::ePreinitialized
    };
    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    let [image, allocation] = device.createImage(imageCreateInfo, allocationCreateInfo);
    let data = device.mapMemory<uint32_t>(allocation);

    stagingTexture = {
        image,
        allocation,
        vk::ImageView(),
        TTauri::PixelMap<uint32_t>{data.data(), to_signed(imageCreateInfo.extent.width), to_signed(imageCreateInfo.extent.height)}
    };

    vk::SamplerCreateInfo const samplerCreateInfo = {
        vk::SamplerCreateFlags(),
        vk::Filter::eLinear, // magFilter
        vk::Filter::eLinear, // minFilter
        vk::SamplerMipmapMode::eNearest, // mipmapMode
        vk::SamplerAddressMode::eRepeat, // addressModeU
        vk::SamplerAddressMode::eRepeat, // addressModeV
        vk::SamplerAddressMode::eRepeat, // addressModeW
        0.0, // mipLodBias
        FALSE, // anisotropyEnable
        0.0, // maxAnisotropy
        FALSE, // compareEnable
        vk::CompareOp::eNever,
        0.0, // minLod
        0.0, // maxLod
        vk::BorderColor::eFloatTransparentBlack,
        FALSE // unnormazlizedCoordinates
    };
    atlasSampler = device.createSampler(samplerCreateInfo);

    atlasSamplerDescriptorImageInfo = {
        atlasSampler,
        vk::ImageView(),
        vk::ImageLayout::eUndefined
    };

    // There needs to be at least one atlas image, so the array of samplers can point to
    // the single image.
    addAtlasImage();
}

void DeviceShared::teardownAtlas(gsl::not_null<Device_vulkan *> vulkanDevice)
{
    vulkanDevice->destroy(atlasSampler);

    for (const auto &atlasImage: atlasTextures) {
        vulkanDevice->destroy(atlasImage.view);
        vulkanDevice->destroyImage(atlasImage.image, atlasImage.allocation);
    }
    atlasTextures.clear();

    vulkanDevice->unmapMemory(stagingTexture.allocation);
    vulkanDevice->destroyImage(stagingTexture.image, stagingTexture.allocation);
}

}
