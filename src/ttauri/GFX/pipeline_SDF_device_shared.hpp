// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "pipeline_SDF_texture_map.hpp"
#include "pipeline_SDF_atlas_rect.hpp"
#include "pipeline_SDF_specialization_constants.hpp"
#include "../text/font_glyph_ids.hpp"
#include "../required.hpp"
#include "../log.hpp"
#include "../vspan.hpp"
#include "../hash_map.hpp"
#include "../geometry/rectangle.hpp"
#include "../geometry/scale.hpp"
#include "../geometry/transform.hpp"
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>
#include <mutex>
#include <unordered_map>

namespace tt {
class gfx_device_vulkan;
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

    gfx_device_vulkan const &device;

    vk::ShaderModule vertexShaderModule;
    vk::ShaderModule fragmentShaderModule;

    specialization_constants specializationConstants;
    std::vector<vk::SpecializationMapEntry> fragmentShaderSpecializationMapEntries;
    vk::SpecializationInfo fragmentShaderSpecializationInfo;
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

    hash_map<font_glyph_ids, atlas_rect> glyphs_in_atlas;
    texture_map stagingTexture;
    std::vector<texture_map> atlasTextures;

    std::array<vk::DescriptorImageInfo, atlasMaximumNrImages> atlasDescriptorImageInfos;
    vk::Sampler atlasSampler;
    vk::DescriptorImageInfo atlasSamplerDescriptorImageInfo;

    point3 atlas_allocation_position = {};
    /// During allocation on a row, we keep track of the tallest glyph.
    int atlasAllocationMaxHeight = 0;

    device_shared(gfx_device_vulkan const &device);
    ~device_shared();

    device_shared(device_shared const &) = delete;
    device_shared &operator=(device_shared const &) = delete;
    device_shared(device_shared &&) = delete;
    device_shared &operator=(device_shared &&) = delete;

    /*! Deallocate vulkan resources.
     * This is called in the destructor of gfx_device_vulkan, therefor we can not use our `std::weak_ptr<gfx_device_vulkan>
     * device`.
     */
    void destroy(gfx_device_vulkan *vulkanDevice);

    /** Allocate an glyph in the atlas.
     * This may allocate an atlas texture, up to atlasMaximumNrImages.
     */
    [[nodiscard]] atlas_rect allocate_rect(extent2 draw_extent, scale2 draw_scale) noexcept;

    void drawInCommandBuffer(vk::CommandBuffer &commandBuffer);

    /** Once drawing in the staging pixmap is completed, you can upload it to the atlas.
     * This will transition the stating texture to 'source' and the atlas to 'destination'.
     */
    void uploadStagingPixmapToAtlas(atlas_rect const &location);

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
    aarectangle get_bounding_box(font_glyph_ids const &glyphs) const noexcept;

    /** Place vertices for a single glyph.
     *
     * @param vertices The list of vertices to add to.
     * @param clipping_rectangle The rectangle to clip the glyph.
     * @param box The rectangle of the glyph in window coordinates. The box's size must be the size
     *            of the glyph's bounding box times @a glyph_size.
     * @param glyphs The font-id, composed-glyphs to render
     * @param color The color of the glyph.
     */
    void place_vertices(
        vspan<vertex> &vertices,
        aarectangle const &clipping_rectangle,
        quad const &box,
        font_glyph_ids const &glyphs,
        color color) noexcept
    {
        if (_place_vertices(vertices, clipping_rectangle, box, glyphs, color)) {
            prepareAtlasForRendering();
        }
    }

    /** Draw the text on the screen.
     *
     * @param vertices The vertices to draw the glyphs to.
     * @param clipping_rectangle The clipping rectangle in screen space where glyphs should be cut off.
     * @param transform The 2D transformation to move and rotate the box to the correct position on screen.
     * @param text The box of text to draw
     */
    void place_vertices(
        vspan<vertex> &vertices,
        aarectangle const &clipping_rectangle,
        geo::transformer auto const &transform,
        shaped_text const &text) noexcept
    {
        auto atlas_was_updated = false;

        for (ttlet &attr_glyph : text) {
            ttlet glyph_added = _place_vertices(vertices, clipping_rectangle, transform, attr_glyph);
            atlas_was_updated = atlas_was_updated or glyph_added;
        }

        if (atlas_was_updated) {
            prepareAtlasForRendering();
        }
    }

    /** Draw the text on the screen.
     *
     * @param vertices The vertices to draw the glyphs to.
     * @param clipping_rectangle The clipping rectangle in screen space where glyphs should be cut off.
     * @param transform The 2D transformation to move and rotate the box to the correct position on screen.
     * @param text The box of text to draw
     * @param color Override the color of the text to draw.
     */
    void place_vertices(
        vspan<vertex> &vertices,
        aarectangle const &clipping_rectangle,
        geo::transformer auto const &transform,
        shaped_text const &text,
        color color) noexcept
    {
        auto atlas_was_updated = false;

        for (ttlet &attr_glyph : text) {
            ttlet glyph_added = _place_vertices(vertices, clipping_rectangle, transform, attr_glyph, color);
            atlas_was_updated = atlas_was_updated or glyph_added;
        }

        if (atlas_was_updated) {
            prepareAtlasForRendering();
        }
    }

private:
    void buildShaders();
    void teardownShaders(gfx_device_vulkan *vulkanDevice);
    void addAtlasImage();
    void buildAtlas();
    void teardownAtlas(gfx_device_vulkan *vulkanDevice);

    bool _place_vertices(
        vspan<vertex> &vertices,
        aarectangle const &clipping_rectangle,
        quad const &box,
        font_glyph_ids const &glyphs,
        color color) noexcept;

    bool _place_vertices(
        vspan<vertex> &vertices,
        aarectangle clipping_rectangle,
        geo::transformer auto const &transform,
        attributed_glyph const &attr_glyph,
        color color) noexcept
    {
        if (not is_visible(attr_glyph.general_category)) {
            return false;
        }

        ttlet bounding_box = transform * attr_glyph.boundingBox();
        return _place_vertices(vertices, clipping_rectangle, bounding_box, attr_glyph.glyphs, color);
    }

    bool _place_vertices(
        vspan<vertex> &vertices,
        aarectangle const &clipping_rectangle,
        geo::transformer auto const &transform,
        attributed_glyph const &attr_glyph) noexcept
    {
        return _place_vertices(vertices, clipping_rectangle, transform, attr_glyph, attr_glyph.style.color);
    }

    atlas_rect const &add_glyph_to_atlas(decltype(glyphs_in_atlas)::iterator it) noexcept;

    /**
     * @return The Atlas rectangle and true if a new glyph was added to the atlas.
     */
    template<typename Glyph>
    tt_force_inline std::pair<atlas_rect const *, bool> get_glyph_from_atlas(Glyph &&glyph) noexcept
    {
        auto i = glyphs_in_atlas.find_or_create(std::forward<Glyph>(glyph));
        if (i != glyphs_in_atlas.cend()) {
            [[likely]] return {&i->value(), false};

        } else {
            return {&add_glyph_to_atlas(i), false};
        }
    }
};

} // namespace tt::pipeline_SDF
