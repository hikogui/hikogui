// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/PipelineSDF_TextureMap.hpp"
#include "TTauri/GUI/PipelineSDF_AtlasRect.hpp"
#include "TTauri/GUI/Device_forward.hpp"
#include "TTauri/Text/FontGlyphIDs.hpp"
#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/logger.hpp"
#include "TTauri/Foundation/vspan.hpp"
#include "TTauri/Foundation/ivec.hpp"
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>
#include <mutex>
#include <unordered_map>

namespace TTauri {
template<typename T> struct PixelMap;
class vec;
class mat;
}

namespace TTauri::Text {
class ShapedText;
}

namespace TTauri::GUI::PipelineSDF {

struct Image;

struct DeviceShared final {
    // Studies in China have shown that literate individuals know and use between 3,000 and 4,000 characters.
    // Handle up to 4096 characters with a 16 x 1024 x 1024, 16 x 1 MByte
    static constexpr int atlasImageWidth = 1024; // 16 characters, of 64 pixels wide.
    static constexpr int atlasImageHeight = 1024; // 16 characters, of 64 pixels height.
    static constexpr int atlasMaximumNrImages = 16; // 16 * 512 characters, of 64x64 pixels.
    static constexpr int stagingImageWidth = 128; // maximum size of character that can be uploaded is 128x128
    static constexpr int stagingImageHeight = 128;

    static constexpr float fontSize = 28.0f;
    static constexpr float drawBorder = SDF8::max_distance;
    static constexpr float scaledDrawBorder = drawBorder / fontSize;

    Device const &device;

    vk::ShaderModule vertexShaderModule;
    vk::ShaderModule fragmentShaderModule;

    std::array<vk::SpecializationMapEntry,1> fragmentShaderSpecializationEntries;
    vk::SpecializationInfo fragmentShaderSpecializationInfo;

    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

    std::unordered_map<Text::FontGlyphIDs,AtlasRect> glyphs_in_atlas;
    TextureMap stagingTexture;
    std::vector<TextureMap> atlasTextures;

    std::array<vk::DescriptorImageInfo, atlasMaximumNrImages> atlasDescriptorImageInfos;
    vk::Sampler atlasSampler;
    vk::DescriptorImageInfo atlasSamplerDescriptorImageInfo;

    ivec atlasAllocationPosition = {};
    /// During allocation on a row, we keep track of the tallest glyph.
    int atlasAllocationMaxHeight = 0;


    DeviceShared(Device const &device);
    ~DeviceShared();

    DeviceShared(DeviceShared const &) = delete;
    DeviceShared &operator=(DeviceShared const &) = delete;
    DeviceShared(DeviceShared &&) = delete;
    DeviceShared &operator=(DeviceShared &&) = delete;

    /*! Deallocate vulkan resources.
    * This is called in the destructor of Device_vulkan, therefor we can not use our `std::weak_ptr<Device_vulkan> device`.
    */
    void destroy(gsl::not_null<Device *> vulkanDevice);

    /** Allocate an glyph in the atlas.
     * This may allocate an atlas texture, up to atlasMaximumNrImages.
     */
    [[nodiscard]] AtlasRect allocateRect(ivec extent) noexcept;

    void drawInCommandBuffer(vk::CommandBuffer &commandBuffer);

    /** Once drawing in the staging pixmap is completed, you can upload it to the atlas.
     * This will transition the stating texture to 'source' and the atlas to 'destination'.
     */
    void uploadStagingPixmapToAtlas(AtlasRect location);

    /** This will transition the staging texture to 'general' for writing by the CPU.
    */
    void prepareStagingPixmapForDrawing();

    /** This will transition the atlas to 'shader-read'.
     */
    void prepareAtlasForRendering();

    /** Prepare the atlas for drawing a text.
     */
    void prepareAtlas(Text::ShapedText const &text) noexcept;

    /** Draw the text on the screen.
     * @param text The box of text to draw
     * @param transform The 2D transformation to move and rotate the box to the correct position on screen.
     * @param clippingRectangle The clipping rectangle in screen space where glyphs should be cut off.
     * @param depth The depth where this text is drawn
     * @param vertices The vertices to draw the glyphs to.
     */
    void placeVertices(vspan<Vertex> &vertices, Text::ShapedText const &text, mat transform, rect clippingRectangle) noexcept;

private:
    void buildShaders();
    void teardownShaders(gsl::not_null<Device_vulkan *> vulkanDevice);
    void addAtlasImage();
    void buildAtlas();
    void teardownAtlas(gsl::not_null<Device_vulkan *> vulkanDevice);
};

}
