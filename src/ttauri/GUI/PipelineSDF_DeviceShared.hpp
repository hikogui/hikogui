// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "PipelineSDF_TextureMap.hpp"
#include "PipelineSDF_AtlasRect.hpp"
#include "PipelineSDF_SpecializationConstants.hpp"
#include "GUIDevice_forward.hpp"
#include "../text/FontGlyphIDs.hpp"
#include "../required.hpp"
#include "../logger.hpp"
#include "../vspan.hpp"
#include "../ivec.hpp"
#include "../rect.hpp"
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>
#include <mutex>
#include <unordered_map>

namespace tt {
template<typename T> struct PixelMap;
class vec;
class mat;
}

namespace tt {
class ShapedText;
struct AttributedGlyph;
}

namespace tt::PipelineSDF {

struct Image;
struct Vertex;

struct DeviceShared final {
    // Studies in China have shown that literate individuals know and use between 3,000 and 4,000 characters.
    // Handle up to 4096 characters with a 16 x 1024 x 1024, 16 x 1 MByte
    static constexpr int atlasImageWidth = 1024; // 16 characters, of 64 pixels wide.
    static constexpr int atlasImageHeight = 1024; // 16 characters, of 64 pixels height.
    static_assert(atlasImageWidth == atlasImageHeight, "needed for fwidth(textureCoord)");

    static constexpr int atlasMaximumNrImages = 16; // 16 * 512 characters, of 64x64 pixels.
    static constexpr int stagingImageWidth = 128; // maximum size of character that can be uploaded is 128x128
    static constexpr int stagingImageHeight = 128;

    static constexpr float atlasTextureCoordinateMultiplier = 1.0f / atlasImageWidth;
    static constexpr float drawFontSize = 28.0f;
    static constexpr float drawBorder = SDF8::max_distance;
    static constexpr float scaledDrawBorder = drawBorder / drawFontSize;

    GUIDevice const &device;

    vk::ShaderModule vertexShaderModule;
    vk::ShaderModule fragmentShaderModule;

    SpecializationConstants specializationConstants;
    std::vector<vk::SpecializationMapEntry> fragmentShaderSpecializationMapEntries;
    vk::SpecializationInfo fragmentShaderSpecializationInfo;
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

    std::unordered_map<FontGlyphIDs,AtlasRect> glyphs_in_atlas;
    TextureMap stagingTexture;
    std::vector<TextureMap> atlasTextures;

    std::array<vk::DescriptorImageInfo, atlasMaximumNrImages> atlasDescriptorImageInfos;
    vk::Sampler atlasSampler;
    vk::DescriptorImageInfo atlasSamplerDescriptorImageInfo;

    ivec atlasAllocationPosition = {};
    /// During allocation on a row, we keep track of the tallest glyph.
    int atlasAllocationMaxHeight = 0;


    DeviceShared(GUIDevice const &device);
    ~DeviceShared();

    DeviceShared(DeviceShared const &) = delete;
    DeviceShared &operator=(DeviceShared const &) = delete;
    DeviceShared(DeviceShared &&) = delete;
    DeviceShared &operator=(DeviceShared &&) = delete;

    /*! Deallocate vulkan resources.
    * This is called in the destructor of GUIDevice_vulkan, therefor we can not use our `std::weak_ptr<GUIDevice_vulkan> device`.
    */
    void destroy(GUIDevice *vulkanDevice);

    /** Allocate an glyph in the atlas.
     * This may allocate an atlas texture, up to atlasMaximumNrImages.
     */
    [[nodiscard]] AtlasRect allocateRect(vec drawExtent) noexcept;

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
    void prepareAtlas(ShapedText const &text) noexcept;

    /** Get the bounding box, including draw border of a glyph.
     */
    static aarect getBoundingBox(FontGlyphIDs const &glyphs) noexcept;
    
    /** Place vertices for a single glyph.
    * @param vertices The list of vertices to add to.
    * @param glyphs The font-id, composed-glyphs to render
    * @param box The rectangle of the glyph in window coordinates; including the draw border.
    * @param color The color of the glyph.
    * @param clippingRectangle The rectangle to clip the glyph.
    */
    void placeVertices(vspan<Vertex> &vertices, FontGlyphIDs const &glyphs, rect box, vec color, aarect clippingRectangle) noexcept;

    /** Draw the text on the screen.
     * @param text The box of text to draw
     * @param transform The 2D transformation to move and rotate the box to the correct position on screen.
     * @param clippingRectangle The clipping rectangle in screen space where glyphs should be cut off.
     * @param vertices The vertices to draw the glyphs to.
     */
    void placeVertices(vspan<Vertex> &vertices, ShapedText const &text, mat transform, aarect clippingRectangle) noexcept;

    /** Draw the text on the screen.
    * @param text The box of text to draw
    * @param transform The 2D transformation to move and rotate the box to the correct position on screen.
    * @param clippingRectangle The clipping rectangle in screen space where glyphs should be cut off.
    * @param vertices The vertices to draw the glyphs to.
    * @param color Override the color of the text to draw.
    */
    void placeVertices(vspan<Vertex> &vertices, ShapedText const &text, mat transform, aarect clippingRectangle, vec color) noexcept;

private:
    void buildShaders();
    void teardownShaders(GUIDevice_vulkan *vulkanDevice);
    void addAtlasImage();
    void buildAtlas();
    void teardownAtlas(GUIDevice_vulkan *vulkanDevice);

    /** Place vertices for a single glyph.
     * This function will not execute prepareAtlasForRendering().
     *
     * @param vertices The list of vertices to add to.
     * @param glyphs The font-id, composed-glyphs to render
     * @param box The rectangle of the glyph in window coordinates; including the draw border.
     * @param color The color of the glyph.
     * @param clippingRectangle The rectangle to clip the glyph.
     * @return True if the glyph was added to the atlas.
     */
    [[nodiscard]] bool _placeVertices(vspan<Vertex> &vertices, FontGlyphIDs const &glyphs, rect box, vec color, aarect clippingRectangle) noexcept;

    /** Place an single attributed glyph.
    * This function will not execute prepareAtlasForRendering().
    *
    * @param vertices The list of vertices to add to.
    * @param attr_glyph The attributed glyph; scaled and positioned.
    * @param transform Extra transformation on the glyph.
    * @param clippingRectangle The rectangle to clip the glyph.
    * @return True if the glyph was added to the atlas.
    */
    [[nodiscard]] bool _placeVertices(vspan<Vertex> &vertices, AttributedGlyph const &attr_glyph, mat transform, aarect clippingRectangle) noexcept;

    /** Place an single attributed glyph.
    * This function will not execute prepareAtlasForRendering().
    *
    * @param vertices The list of vertices to add to.
    * @param attr_glyph The attributed glyph; scaled and positioned.
    * @param transform Extra transformation on the glyph.
    * @param clippingRectangle The rectangle to clip the glyph.
    * @param color Override the color from the glyph style.
    * @return True if the glyph was added to the atlas.
    */
    [[nodiscard]] bool _placeVertices(vspan<Vertex> &vertices, AttributedGlyph const &attr_glyph, mat transform, aarect clippingRectangle, vec color) noexcept;

    AtlasRect addGlyphToAtlas(FontGlyphIDs glyph) noexcept;

    /**
     * @return The Atlas rectangle and true if a new glyph was added to the atlas.
     */
    std::pair<AtlasRect,bool> getGlyphFromAtlas(FontGlyphIDs glyph) noexcept;

};

}
