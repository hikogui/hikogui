// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "pipeline_SDF_texture_map.hpp"
#include "pipeline_SDF_specialization_constants.hpp"
#include "../text/glyph_ids.hpp"
#include "../text/glyph_atlas_info.hpp"
#include "../utility.hpp"
#include "../log.hpp"
#include "../vector_span.hpp"
#include "../geometry/rectangle.hpp"
#include "../geometry/scale.hpp"
#include "../geometry/transform.hpp"
#include "../color/quad_color.hpp"
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>
#include <mutex>
#include <unordered_map>

namespace hi::inline v1 {
template<typename T>
class pixel_map;
class mat;
class gfx_device_vulkan;
class shaped_text;
struct attributed_glyph;

namespace pipeline_SDF {
struct Image;
struct vertex;

struct device_shared final {
    // Studies in China have shown that literate individuals know and use between 3,000 and 4,000 characters.
    // Handle up to 7 * 7 * 128 == 6321 characters with a 16 x 1024 x 1024, 16 x 1 MByte
    //
    // For latin characters we can store about 7 * 12 == 84 characters in a single image, which is enough
    // for the full alpha numeric range that an application will use.

    static constexpr int atlasImageWidth = 256; // 7-12 characters, of 34 pixels wide.
    static constexpr int atlasImageHeight = 256; // 7 characters, of 34 pixels height.
    static_assert(atlasImageWidth == atlasImageHeight, "needed for fwidth(textureCoord)");

    static constexpr int atlasMaximumNrImages = 128; // 128 * 49 characters.
    static constexpr int stagingImageWidth = 64; // One 'em' is 28 pixels, with edges 34 pixels.
    static constexpr int stagingImageHeight = 64;

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
    [[nodiscard]] glyph_atlas_info allocate_rect(extent2 draw_extent, scale2 draw_scale) noexcept;

    void drawInCommandBuffer(vk::CommandBuffer &commandBuffer);

    /** Once drawing in the staging pixmap is completed, you can upload it to the atlas.
     * This will transition the stating texture to 'source' and the atlas to 'destination'.
     */
    void uploadStagingPixmapToAtlas(glyph_atlas_info const &location);

    /** This will transition the staging texture to 'general' for writing by the CPU.
     */
    void prepareStagingPixmapForDrawing();

    /** This will transition the atlas to 'shader-read'.
     */
    void prepare_atlas_for_rendering();

    /** Prepare the atlas for drawing a text.
     */
    void prepareAtlas(shaped_text const &text) noexcept;

    /** Get the bounding box, including draw border of a glyph.
     */
    aarectangle get_bounding_box(glyph_ids const &glyphs) const noexcept;

    /** Place vertices for a single glyph.
     *
     * @param vertices The list of vertices to add to.
     * @param clipping_rectangle The rectangle to clip the glyph.
     * @param box The rectangle of the glyph in window coordinates. The box's size must be the size
     *            of the glyph's bounding box times @a glyph_size.
     * @param glyphs The font-id, composed-glyphs to render
     * @param colors The color of each corner of the glyph.
     * @return True is atlas was updated.
     */
    bool place_vertices(
        vector_span<vertex> &vertices,
        aarectangle const &clipping_rectangle,
        quad const &box,
        glyph_ids const &glyphs,
        quad_color colors) noexcept;

private:
    void buildShaders();
    void teardownShaders(gfx_device_vulkan *vulkanDevice);
    void addAtlasImage();
    void buildAtlas();
    void teardownAtlas(gfx_device_vulkan *vulkanDevice);
    void add_glyph_to_atlas(glyph_ids const &glyph, glyph_atlas_info &info) noexcept;

    /**
     * @return The Atlas rectangle and true if a new glyph was added to the atlas.
     */
    hi_force_inline std::pair<glyph_atlas_info const *, bool> get_glyph_from_atlas(glyph_ids const &glyph) noexcept
    {
        auto &info = glyph.atlas_info();

        if (info) [[likely]] {
            return {&info, false};

        } else {
            add_glyph_to_atlas(glyph, info);
            return {&info, true};
        }
    }
};

} // namespace pipeline_SDF
} // namespace hi::inline v1
