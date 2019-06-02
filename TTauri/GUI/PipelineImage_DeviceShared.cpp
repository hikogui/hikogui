// Copyright 2019 Pokitec
// All rights reserved.

#include "PipelineImage.hpp"
#include "PipelineImage_DeviceShared.hpp"
#include "PipelineImage_Image.hpp"
#include "Device.hpp"
#include "TTauri/all.hpp"
#include <glm/gtx/vec_swizzle.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/range/combine.hpp>
#include <array>

#define BINARY_ASSETS_PipelineImage_frag_spv_IMPL
#include "PipelineImage.frag.spv.hpp"
#define BINARY_ASSETS_PipelineImage_vert_spv_IMPL
#include "PipelineImage.vert.spv.hpp"

namespace TTauri::GUI::PipelineImage {

using namespace std;

DeviceShared::DeviceShared(const std::shared_ptr<Device_vulkan> device) :
    device(move(device))
{
    buildIndexBuffer();
    buildShaders();
    buildAtlas();
}

DeviceShared::~DeviceShared()
{
}

void DeviceShared::destroy(gsl::not_null<Device_vulkan *> vulkanDevice)
{
    teardownIndexBuffer(vulkanDevice);
    teardownShaders(vulkanDevice);
    teardownAtlas(vulkanDevice);
}

std::vector<Page> DeviceShared::getFreePages(size_t const nrPages)
{
    while (nrPages > atlasFreePages.size()) {
        addAtlasImage();
    }

    auto pages = std::vector<Page>();
    for (size_t i = 0; i < nrPages; i++) {
        auto const page = atlasFreePages.back();
        pages.push_back(page);
        atlasFreePages.pop_back();
    }
    return pages;
}

std::shared_ptr<Image> DeviceShared::retainImage(const std::string &key, u64extent2 const extent)
{
    auto const i = viewImages.find(key);
    if (i != viewImages.end()) {
        auto image = (*i).second;
        image->retainCount++;
        return image;
    }

    auto const pageExtent = u64extent2{
        (extent.width() + (Page::width - 1)) / Page::width,
        (extent.height() + (Page::height - 1)) / Page::height
    };

    auto const pages = getFreePages(pageExtent.x * pageExtent.y);
    auto image = TTauri::make_shared<Image>(key, extent, pageExtent, pages);
    viewImages.insert_or_assign(key, image);
    return image;
}

void DeviceShared::releaseImage(const std::shared_ptr<Image> &image)
{
    if (--image->retainCount == 0) {
        auto const i = viewImages.find(image->key);
        if (i != viewImages.end()) {
            viewImages.erase(i);
        } else {
            BOOST_THROW_EXCEPTION(Error());
        }
    }
}

void DeviceShared::exchangeImage(std::shared_ptr<Image> &image, const std::string &key, const u64extent2 extent)
{
    if (image && image->key == key) {
        return;
    }

    if (image) {
        releaseImage(image);
    }
    
    image = retainImage(key, extent);
}

TTauri::Draw::PixelMap<uint32_t> DeviceShared::getStagingPixelMap()
{
    auto vulkanDevice = device.lock();
    stagingTexture.transitionLayout(*vulkanDevice, vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eGeneral);

    auto const textureWithoutBorder = stagingTexture.pixelMap.submap(
        Page::border, Page::border,
        stagingImageWidth - 2 * Page::border, stagingImageHeight - 2 * Page::border
    );
    return textureWithoutBorder;
}

void DeviceShared::updateAtlasWithStagingPixelMap(const Image &image)
{
    auto const vulkanDevice = device.lock();

    // Start with the actual image inside the stagingImage.
    auto rectangle = u64rect2{
        {Page::border, Page::border},
        image.extent
    };
    // Add one pixel of border around the actual image and keep extending
    // until the full border with is finished.
    for (size_t b = 0; b < Page::border; b++) {
        rectangle.offset -= glm::u64vec2(1, 1);
        rectangle.extent += glm::u64vec2(2, 2);

        auto pixelMap = stagingTexture.pixelMap.submap(rectangle);
        TTauri::Draw::add1PixelTransparentBorder(pixelMap);
    }

    // Flush the given image, included the border.
    vulkanDevice->flushAllocation(
        stagingTexture.allocation,
        0,
        ((image.extent.height() + 2 * Page::border) * stagingTexture.pixelMap.stride) * sizeof (uint32_t)
    );

    stagingTexture.transitionLayout(*vulkanDevice, vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eTransferSrcOptimal);

    array<vector<vk::ImageCopy>, atlasMaximumNrImages> regionsToCopyPerAtlasTexture; 
    for (size_t index = 0 ; index < image.pages.size(); index++) {
        auto const page = image.pages.at(index);

        if (page.isFullyTransparent()) {
            // Hole in the image does not need to be rendered.
            continue;
        }

        auto imageRect = image.indexToRect(index);
        // Adjust the position to be inside the stagingImage, excluding its border.
        imageRect.offset += glm::u64vec2(Page::border, Page::border);
        auto const atlasPosition = getAtlasPositionFromPage(page);

        // During copying we want to copy extra pixels around each page, this allows for non-nearest-neighbour sampling
        // on the edge of a page.
        imageRect.offset -= glm::u64vec2(Page::border, Page::border);
        imageRect.extent += glm::u64vec2(Page::border*2, Page::border*2);
        auto const atlasOffset = xy(atlasPosition) - glm::u64vec2(Page::border, Page::border);

        auto &regionsToCopy = regionsToCopyPerAtlasTexture.at(atlasPosition.z);
        regionsToCopy.push_back({
            { vk::ImageAspectFlagBits::eColor, 0, 0, 1 },
            { boost::numeric_cast<int32_t>(imageRect.offset.x), boost::numeric_cast<int32_t>(imageRect.offset.y), 0 },
            { vk::ImageAspectFlagBits::eColor, 0, 0, 1 },
            { boost::numeric_cast<int32_t>(atlasOffset.x), boost::numeric_cast<int32_t>(atlasOffset.y), 0 },
            { boost::numeric_cast<uint32_t>(imageRect.extent.width()), boost::numeric_cast<uint32_t>(imageRect.extent.height()), 1}
        });
    }

    for (size_t atlasTextureIndex = 0; atlasTextureIndex < atlasTextures.size(); atlasTextureIndex++) {
        auto const &regionsToCopy = regionsToCopyPerAtlasTexture.at(atlasTextureIndex);
        if (regionsToCopy.size() == 0) {
            continue;
        }

        auto &atlasTexture = atlasTextures.at(atlasTextureIndex);
        atlasTexture.transitionLayout(*vulkanDevice, vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eTransferDstOptimal);

        vulkanDevice->copyImage(stagingTexture.image, vk::ImageLayout::eTransferSrcOptimal, atlasTexture.image, vk::ImageLayout::eTransferDstOptimal, regionsToCopy);
    }
}

void DeviceShared::prepareAtlasForRendering()
{
    auto const vulkanDevice = device.lock();

    for (auto &atlasTexture: atlasTextures) {
        atlasTexture.transitionLayout(*vulkanDevice, vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eShaderReadOnlyOptimal);
    }
}

void DeviceShared::drawInCommandBuffer(vk::CommandBuffer &commandBuffer)
{
    auto const vulkanDevice = device.lock();

    commandBuffer.bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint16);
}


void DeviceShared::buildIndexBuffer()
{
    auto const vulkanDevice = device.lock();

    // Create vertex index buffer
    {
        vk::BufferCreateInfo const bufferCreateInfo = {
            vk::BufferCreateFlags(),
            sizeof (uint16_t) * PipelineImage::PipelineImage::maximumNumberOfIndices,
            vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
            vk::SharingMode::eExclusive
        };
        VmaAllocationCreateInfo allocationCreateInfo = {};
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        tie(indexBuffer, indexBufferAllocation) = vulkanDevice->createBuffer(bufferCreateInfo, allocationCreateInfo);
    }

    // Fill in the vertex index buffer, using a staging buffer, then copying.
    {
        // Create staging vertex index buffer.
        vk::BufferCreateInfo const bufferCreateInfo = {
            vk::BufferCreateFlags(),
            sizeof (uint16_t) * PipelineImage::PipelineImage::maximumNumberOfIndices,
            vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferSrc,
            vk::SharingMode::eExclusive
        };
        VmaAllocationCreateInfo allocationCreateInfo = {};
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
        auto const [stagingVertexIndexBuffer, stagingVertexIndexBufferAllocation] = vulkanDevice->createBuffer(bufferCreateInfo, allocationCreateInfo);

        // Initialize indices.
        auto const stagingVertexIndexBufferData = vulkanDevice->mapMemory<uint16_t>(stagingVertexIndexBufferAllocation);
        for (size_t i = 0; i < PipelineImage::PipelineImage::maximumNumberOfIndices; i++) {
            auto const vertexInRectangle = i % 6;
            auto const rectangleNr = i / 6;
            auto const rectangleBase = rectangleNr * 4;

            switch (vertexInRectangle) {
            case 0: gsl::at(stagingVertexIndexBufferData, i) = boost::numeric_cast<uint16_t>(rectangleBase + 0); break;
            case 1: gsl::at(stagingVertexIndexBufferData, i) = boost::numeric_cast<uint16_t>(rectangleBase + 1); break;
            case 2: gsl::at(stagingVertexIndexBufferData, i) = boost::numeric_cast<uint16_t>(rectangleBase + 2); break;
            case 3: gsl::at(stagingVertexIndexBufferData, i) = boost::numeric_cast<uint16_t>(rectangleBase + 2); break;
            case 4: gsl::at(stagingVertexIndexBufferData, i) = boost::numeric_cast<uint16_t>(rectangleBase + 1); break;
            case 5: gsl::at(stagingVertexIndexBufferData, i) = boost::numeric_cast<uint16_t>(rectangleBase + 3); break;
            }
        }
        vulkanDevice->flushAllocation(stagingVertexIndexBufferAllocation, 0, VK_WHOLE_SIZE);
        vulkanDevice->unmapMemory(stagingVertexIndexBufferAllocation);

        // Copy indices to vertex index buffer.
        auto commands = vulkanDevice->allocateCommandBuffers({
            vulkanDevice->graphicsCommandPool, 
            vk::CommandBufferLevel::ePrimary, 
            1
            }).at(0);
        commands.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
        commands.copyBuffer(stagingVertexIndexBuffer, indexBuffer, {{0, 0, sizeof (uint16_t) * PipelineImage::PipelineImage::maximumNumberOfIndices}});
        commands.end();

        vector<vk::CommandBuffer> const commandBuffersToSubmit = { commands };
        vector<vk::SubmitInfo> const submitInfo = { { 0, nullptr, nullptr, boost::numeric_cast<uint32_t>(commandBuffersToSubmit.size()), commandBuffersToSubmit.data(), 0, nullptr } };
        vulkanDevice->graphicsQueue.submit(submitInfo, vk::Fence());
        vulkanDevice->graphicsQueue.waitIdle();

        vulkanDevice->freeCommandBuffers(vulkanDevice->graphicsCommandPool, {commands});
        vulkanDevice->destroyBuffer(stagingVertexIndexBuffer, stagingVertexIndexBufferAllocation);
    }
}

void DeviceShared::teardownIndexBuffer(gsl::not_null<Device_vulkan *> vulkanDevice)
{
    vulkanDevice->destroyBuffer(indexBuffer, indexBufferAllocation);
}

void DeviceShared::buildShaders()
{
    auto vulkanDevice = device.lock();

    vertexShaderModule = vulkanDevice->loadShader(BinaryAssets::u32PipelineImage_vert_spv, sizeof(BinaryAssets::PipelineImage_vert_spv));
    fragmentShaderModule = vulkanDevice->loadShader(BinaryAssets::u32PipelineImage_frag_spv, sizeof(BinaryAssets::PipelineImage_frag_spv));

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
    auto vulkanDevice = device.lock();
    auto const currentImageIndex = atlasTextures.size();

    // Create atlas image
    vk::ImageCreateInfo const imageCreateInfo = {
        vk::ImageCreateFlags(),
        vk::ImageType::e2D,
        vk::Format::eR8G8B8A8Unorm,
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

    auto const [atlasImage, atlasImageAllocation] = vulkanDevice->createImage(imageCreateInfo, allocationCreateInfo);

    auto const atlasImageView = vulkanDevice->createImageView({
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
    size_t const pageOffset = currentImageIndex * atlasNrPagesPerImage;
    for (size_t i = 0; i < atlasNrPagesPerImage; i++) {
            atlasFreePages.push_back({pageOffset + i});
    }

    // Build image descriptor info.
    for (size_t i = 0; i < atlasDescriptorImageInfos.size(); i++) {
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
    auto vulkanDevice = device.lock();

    // Create staging image
    vk::ImageCreateInfo const imageCreateInfo = {
        vk::ImageCreateFlags(),
        vk::ImageType::e2D,
        vk::Format::eR8G8B8A8Unorm,
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
    auto const [image, allocation] = vulkanDevice->createImage(imageCreateInfo, allocationCreateInfo);
    auto const data = vulkanDevice->mapMemory<uint32_t>(allocation);
    auto const pixelMap = TTauri::Draw::PixelMap<uint32_t>{data, imageCreateInfo.extent.width, imageCreateInfo.extent.height};

    stagingTexture = { image, allocation, vk::ImageView(), pixelMap };

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
    atlasSampler = vulkanDevice->createSampler(samplerCreateInfo);

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
