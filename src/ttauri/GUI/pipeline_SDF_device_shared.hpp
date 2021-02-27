// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "pipeline_SDF_texture_map.hpp"
#include "pipeline_SDF_atlas_rect.hpp"
#include "pipeline_SDF_specialization_constants.hpp"
#include "../text/font_glyph_ids.hpp"
#include "../required.hpp"
#include "../logger.hpp"
#include "../vspan.hpp"
#include "../numeric_array.hpp"
#include "../rect.hpp"
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>
#include <mutex>
#include <unordered_map>

namespace tt {
class gui_device_vulkan;
template<typename T>
class pixel_map;
class mat;
} // namespace tt

namespace tt {
class shaped_text;
struct attributed_glyph;
} // namespace tt

namespace tt::pipeline_SDF {

struct Image;
struct vertex;

struct device_shared final {
    // Studies in China have shown that literate individuals know and use between 3,000 and 4,000 characters.
    // Handle up to 4096 characters with a 16 x 1024 x 1024, 16 x 1 MByte
    static constexpr int atlasImageWidth = 1024; // 16 characters, of 64 pixels wide.
    static constexpr int atlasImageHeight = 1024; // 16 characters, of 64 pixels height.
    static_assert(atlasImageWidth == atlasImageHeight, "needed for fwidth(textureCoord)");

    static constexpr int atlasMaximumNrImages = 16; // 16 * 512 characters, of 64x64 pixels.
    static constexpr int stagingImageWidth = 128; // maximum size of character that can be uploaded is 128x128
    static constexpr int stagingImageHeight = 128;

    static constexpr float atlasTextureCoordinateMultiplier = 1.0f / atlasImageWidth;
    static constexpr float drawfontSize = 28.0f;
    static constexpr float drawBorder = sdf_r8::max_distance;
    static constexpr float scaledDrawBorder = drawBorder / drawfontSize;

    gui_device_vulkan const &device;

    vk::ShaderModule vertexShaderModule;
    vk::ShaderModule fragmentShaderModule;

    specialization_constants specializationConstants;
    std::vector<vk::SpecializationMapEntry> fragmentShaderSpecializationMapEntries;
    vk::SpecializationInfo fragmentShaderSpecializationInfo;
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

    std::unordered_map<font_glyph_ids, atlas_rect> glyphs_in_atlas;
    texture_map stagingTexture;
    std::vector<texture_map> atlasTextures;

    std::array<vk::DescriptorImageInfo, atlasMaximumNrImages> atlasDescriptorImageInfos;
    vk::Sampler atlasSampler;
    vk::DescriptorImageInfo atlasSamplerDescriptorImageInfo;

    point3 atlas_allocation_position = {};
    /// During allocation on a row, we keep track of the tallest glyph.
    int atlasAllocationMaxHeight = 0;

    device_shared(gui_device_vulkan const &device);
    ~device_shared();

    device_shared(device_shared const &) = delete;
    device_shared &operator=(device_shared const &) = delete;
    device_shared(device_shared &&) = delete;
    device_shared &operator=(device_shared &&) = delete;

    /*! Deallocate vulkan resources.
     * This is called in the destructor of gui_device_vulkan, therefor we can not use our `std::weak_ptr<gui_device_vulkan>
     * device`.
     */
    void destroy(gui_device_vulkan *vulkanDevice);

    /** Allocate an glyph in the atlas.
     * This may allocate an atlas texture, up to atlasMaximumNrImages.
     */
    [[nodiscard]] atlas_rect allocateRect(extent2 drawExtent) noexcept;

    void drawInCommandBuffer(vk::CommandBuffer &commandBuffer);

    /** Once drawing in the staging pixmap is completed, you can upload it to the atlas.
     * This will transition the stating texture to 'source' and the atlas to 'destination'.
     */
    void uploadStagingPixmapToAtlas(atlas_rect location);

    /** This will transition the staging texture to 'general' for writing by the CPU.
     */
    void prepareStagingPixmapForDrawing();

    /** This will transition the atlas to 'shader-read'.
     */
    void prepareAtlasForRendering();

    /** Prepare the atlas for drawing a text.
     */
    void prepareAtlas(shaped_text const &text) noexcept;

    /** Get the bounding box, including draw border of a glyph.
     */
    static aarect getBoundingBox(font_glyph_ids const &glyphs) noexcept;

    /** Place vertices for a single glyph.
     * @param vertices The list of vertices to add to.
     * @param glyphs The font-id, composed-glyphs to render
     * @param box The rectangle of the glyph in window coordinates; including the draw border.
     * @param color The color of the glyph.
     * @param clippingRectangle The rectangle to clip the glyph.
     */
    void
    place_vertices(vspan<vertex> &vertices, aarect clipping_rectangle, rect box, font_glyph_ids const &glyphs, color color) noexcept;

    /** Draw the text on the screen.
     * @param text The box of text to draw
     * @param transform The 2D transformation to move and rotate the box to the correct position on screen.
     * @param clippingRectangle The clipping rectangle in screen space where glyphs should be cut off.
     * @param vertices The vertices to draw the glyphs to.
     */
    void place_vertices(vspan<vertex> &vertices, aarect clipping_rectangle, matrix3 transform, shaped_text const &text) noexcept;

    /** Draw the text on the screen.
     * @param text The box of text to draw
     * @param transform The 2D transformation to move and rotate the box to the correct position on screen.
     * @param clippingRectangle The clipping rectangle in screen space where glyphs should be cut off.
     * @param vertices The vertices to draw the glyphs to.
     * @param color Override the color of the text to draw.
     */
    void place_vertices(
        vspan<vertex> &vertices,
        aarect clipping_rectangle,
        matrix3 transform,
        shaped_text const &text,
        color color) noexcept;

private:
    void buildShaders();
    void teardownShaders(gui_device_vulkan *vulkanDevice);
    void addAtlasImage();
    void buildAtlas();
    void teardownAtlas(gui_device_vulkan *vulkanDevice);

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
    [[nodiscard]] bool _place_vertices(
        vspan<vertex> &vertices,
        aarect clipping_rectangle,
        rect box,
        font_glyph_ids const &glyphs,
        color color
        ) noexcept;

    /** Place an single attributed glyph.
     * This function will not execute prepareAtlasForRendering().
     *
     * @param vertices The list of vertices to add to.
     * @param attr_glyph The attributed glyph; scaled and positioned.
     * @param transform Extra transformation on the glyph.
     * @param clippingRectangle The rectangle to clip the glyph.
     * @return True if the glyph was added to the atlas.
     */
    [[nodiscard]] bool _place_vertices(
        vspan<vertex> &vertices,
        aarect clippingRectangle,
        matrix3 transform,
        attributed_glyph const &attr_glyph
        ) noexcept;

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
    [[nodiscard]] bool _place_vertices(
        vspan<vertex> &vertices,
        aarect clippingRectangle,
        matrix3 transform,
        attributed_glyph const &attr_glyph,
        color color) noexcept;

    atlas_rect addGlyphToAtlas(font_glyph_ids glyph) noexcept;

    /**
     * @return The Atlas rectangle and true if a new glyph was added to the atlas.
     */
    std::pair<atlas_rect, bool> getGlyphFromAtlas(font_glyph_ids glyph) noexcept;
};

} // namespace tt::pipeline_SDF
