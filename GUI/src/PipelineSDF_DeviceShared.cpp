// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/GUI/PipelineSDF.hpp"
#include "TTauri/GUI/PipelineSDF_DeviceShared.hpp"
#include "TTauri/GUI/Device.hpp"
#include "TTauri/Text/ShapedText.hpp"
#include "TTauri/Foundation/PixelMap.hpp"
#include "TTauri/Foundation/URL.hpp"
#include "TTauri/Foundation/memory.hpp"
#include "TTauri/Foundation/numeric_cast.hpp"
#include "TTauri/Foundation/BezierCurve.hpp"
#include "TTauri/Foundation/ivec.hpp"
#include "TTauri/Foundation/irect.hpp"
#include "TTauri/Foundation/mat.hpp"
#include <array>

namespace TTauri::GUI::PipelineSDF {

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

[[nodiscard]] AtlasRect DeviceShared::allocateRect(ivec extent) noexcept
{
    if (atlasAllocationPosition.y() + extent.y() > atlasImageHeight) {
        atlasAllocationPosition.x(0); 
        atlasAllocationPosition.y(0);
        atlasAllocationPosition.z(atlasAllocationPosition.z() + 1);

        if (atlasAllocationPosition.z() >= atlasMaximumNrImages) {
            LOG_FATAL("PipelineSDF atlas overflow, too many glyphs in use.");
        }

        if (atlasAllocationPosition.z() >= size(atlasTextures)) {
            addAtlasImage();
        }
    }

    if (atlasAllocationPosition.x() + extent.x() > atlasImageWidth) {
        atlasAllocationPosition.x(0);
        atlasAllocationPosition.y(atlasAllocationPosition.y() + atlasAllocationMaxHeight);
    }

    auto r = AtlasRect{atlasAllocationPosition, extent};
    
    atlasAllocationPosition.x(atlasAllocationPosition.x() + extent.x());
    atlasAllocationMaxHeight = std::max(atlasAllocationMaxHeight, extent.y());

    return r;
}


void DeviceShared::uploadStagingPixmapToAtlas(AtlasRect location)
{
    // Flush the given image, included the border.
    device.flushAllocation(
        stagingTexture.allocation,
        0,
        (stagingTexture.pixelMap.height * stagingTexture.pixelMap.stride) * sizeof (SDF8)
    );
    
    stagingTexture.transitionLayout(device, vk::Format::eR8Snorm, vk::ImageLayout::eTransferSrcOptimal);

    array<vector<vk::ImageCopy>, atlasMaximumNrImages> regionsToCopyPerAtlasTexture; 

    auto regionsToCopy = std::vector{vk::ImageCopy{
        { vk::ImageAspectFlagBits::eColor, 0, 0, 1 },
        { 0, 0, 0 },
        { vk::ImageAspectFlagBits::eColor, 0, 0, 1 },
        { numeric_cast<int32_t>(location.atlas_position.x()), numeric_cast<int32_t>(location.atlas_position.y()), 0 },
        { numeric_cast<uint32_t>(location.atlas_extent.x()), numeric_cast<uint32_t>(location.atlas_extent.y()), 1}
    }};

    auto &atlasTexture = atlasTextures.at(location.atlas_position.z());
    atlasTexture.transitionLayout(device, vk::Format::eR8Snorm, vk::ImageLayout::eTransferDstOptimal);

    device.copyImage(stagingTexture.image, vk::ImageLayout::eTransferSrcOptimal, atlasTexture.image, vk::ImageLayout::eTransferDstOptimal, std::move(regionsToCopy));
}

void DeviceShared::prepareStagingPixmapForDrawing()
{
    stagingTexture.transitionLayout(device, vk::Format::eR8Snorm, vk::ImageLayout::eGeneral);
}

void DeviceShared::prepareAtlasForRendering()
{
    for (auto &atlasTexture: atlasTextures) {
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
void DeviceShared::prepareAtlas(Text::ShapedText const &text) noexcept
{
    int count = 0;
    for (let &attr_grapheme: text) {
        let atlas_i = glyphs_in_atlas.find(attr_grapheme.glyphs);
        if (atlas_i != glyphs_in_atlas.end()) {
            continue;
        }

        ++count;

        // We will draw the font at 15 pt into the texture. And we need a border for the texture to
        // allow proper bi-linear interpolation on the edges.
        let extent = ivec{
            numeric_cast<int>(std::ceil(attr_grapheme.metrics.boundingBox.width() * fontSize + drawBorder * 2)),
            numeric_cast<int>(std::ceil(attr_grapheme.metrics.boundingBox.height() * fontSize + drawBorder * 2))
        };

        auto atlas_rect = allocateRect(extent);

        // Now create a path of the combined glyphs. Offset and scale the path so that
        // it is rendered at a fixed font size and that the bounding box of the glyph matches the bounding box in the atlas.
        let offset =
            vec{drawBorder, drawBorder} -
            attr_grapheme.metrics.boundingBox.offset() * fontSize;
        let path = (mat::T(offset) * mat::S(fontSize)) * attr_grapheme.glyphs.get_path();

        // Draw glyphs into staging buffer of the atlas.
        prepareStagingPixmapForDrawing();
        auto pixmap = stagingTexture.pixelMap.submap(irect{ivec{}, extent});
        fill(pixmap, path);
        uploadStagingPixmapToAtlas(atlas_rect);

        // The bounding box is in texture coordinates.
        let atlas_px_offset = static_cast<vec>(atlas_rect.atlas_position.xy00());
        let atlas_px_extent = attr_grapheme.metrics.boundingBox.extent() * fontSize + 2.0f * vec{drawBorder, drawBorder};

        let atlas_tx_multiplier = vec{1.0f / atlasImageWidth, 1.0f / atlasImageHeight};
        let atlas_tx_offset = atlas_px_offset * atlas_tx_multiplier;
        let atlas_tx_extent = atlas_px_extent * atlas_tx_multiplier;
        let atlas_tx_box = rect{atlas_tx_offset, atlas_tx_extent};

        let atlas_z = numeric_cast<float>(atlas_rect.atlas_position.z());
        get<0>(atlas_rect.textureCoords) = atlas_tx_box.corner<0>(atlas_z);
        get<1>(atlas_rect.textureCoords) = atlas_tx_box.corner<1>(atlas_z);
        get<2>(atlas_rect.textureCoords) = atlas_tx_box.corner<2>(atlas_z);
        get<3>(atlas_rect.textureCoords) = atlas_tx_box.corner<3>(atlas_z);

        glyphs_in_atlas[attr_grapheme.glyphs] = atlas_rect;
    }

    if (count != 0) {
        prepareAtlasForRendering();
    }
}

/** Places vertices.
*
* This is the format of a quad.
*
*    2 <-- 3
*    | \   ^
*    |  \  |
*    v   \ |
*    0 --> 1
*/
void DeviceShared::placeVertices(vspan<Vertex> &vertices, Text::ShapedText const &text, mat transform, rect clippingRectangle, float depth) noexcept
{
    for (let &attr_grapheme: text) {
        // Adjust bounding box by adding a border based on the fixed font size.
        let bounding_box = expand(rect{attr_grapheme.metrics.boundingBox}, scaledDrawBorder);

        let vM = transform * attr_grapheme.transform;
        let v0 = vM * bounding_box.corner<0>(depth);
        let v1 = vM * bounding_box.corner<1>(depth);
        let v2 = vM * bounding_box.corner<2>(depth);
        let v3 = vM * bounding_box.corner<3>(depth);

        constexpr float texelSize = 1.0f / fontSize;
        constexpr float texelMaxDistance = texelSize * SDF8::max_distance;

        // Extract the max distance in pixels, after scaling the font.
        let pixelMaxDistance = texelMaxDistance * vM.scaleX();

        // If none of the vertices is inside the clipping rectangle then don't add the
        // quad to the vertex list.
        if (!(
            clippingRectangle.contains(v0) ||
            clippingRectangle.contains(v1) ||
            clippingRectangle.contains(v2) ||
            clippingRectangle.contains(v3)
        )) {
            continue;
        }
        let atlas_i = glyphs_in_atlas.find(attr_grapheme.glyphs);
        ttauri_assume(atlas_i != glyphs_in_atlas.end());

        let &atlas_rect = atlas_i->second;

        let color = vec{attr_grapheme.style.color};
        let shadowSize = attr_grapheme.style.shadow_size > 0.1f ? 
            (0.5f / std::min(attr_grapheme.style.shadow_size, pixelMaxDistance)) :
            -1.0f;

        vertices.emplace_back(v0, clippingRectangle, get<0>(atlas_rect.textureCoords), color, pixelMaxDistance, shadowSize);
        vertices.emplace_back(v1, clippingRectangle, get<1>(atlas_rect.textureCoords), color, pixelMaxDistance, shadowSize);
        vertices.emplace_back(v2, clippingRectangle, get<2>(atlas_rect.textureCoords), color, pixelMaxDistance, shadowSize);
        vertices.emplace_back(v3, clippingRectangle, get<3>(atlas_rect.textureCoords), color, pixelMaxDistance, shadowSize);
    }
}


void DeviceShared::drawInCommandBuffer(vk::CommandBuffer &commandBuffer)
{
    commandBuffer.bindIndexBuffer(device.quadIndexBuffer, 0, vk::IndexType::eUint16);
}

void DeviceShared::buildShaders()
{
    vertexShaderModule = device.loadShader(URL("resource:GUI/PipelineSDF.vert.spv"));
    fragmentShaderModule = device.loadShader(URL("resource:GUI/PipelineSDF.frag.spv"));

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
    //let currentImageIndex = to_signed(atlasTextures.size());

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
        vk::Format::eR8Snorm,
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
    let data = device.mapMemory<SDF8>(allocation);

    stagingTexture = {
        image,
        allocation,
        vk::ImageView(),
        TTauri::PixelMap<SDF8>{data.data(), to_signed(imageCreateInfo.extent.width), to_signed(imageCreateInfo.extent.height)}
    };

    vk::SamplerCreateInfo const samplerCreateInfo = {
        vk::SamplerCreateFlags(),
        vk::Filter::eLinear, // magFilter
        vk::Filter::eLinear, // minFilter
        vk::SamplerMipmapMode::eNearest, // mipmapMode
        vk::SamplerAddressMode::eClampToEdge, // addressModeU
        vk::SamplerAddressMode::eClampToEdge, // addressModeV
        vk::SamplerAddressMode::eClampToEdge, // addressModeW
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
