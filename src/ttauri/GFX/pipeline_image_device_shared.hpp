// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "pipeline_image_texture_map.hpp"
#include "pipeline_image_page.hpp"
#include "../required.hpp"
#include "../rapid/sfloat_rgba16.hpp"
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>
#include <mutex>

namespace tt {
class gfx_device_vulkan;
template<typename T>
class pixel_map;
} // namespace tt

namespace tt::pipeline_image {

struct image;

struct device_shared {
    static constexpr size_t atlas_num_pages_per_axis = 16;
    static constexpr size_t atlas_num_pages_per_image = atlas_num_pages_per_axis * atlas_num_pages_per_axis;
    static constexpr size_t atlas_image_axis_size = atlas_num_pages_per_axis * (page::border + page::size + page::border);
    static constexpr size_t atlas_maximum_num_images = 16;
    static constexpr size_t staging_image_width = 1024;
    static constexpr size_t staging_image_height = 1024;

    gfx_device_vulkan const &device;

    vk::ShaderModule vertex_shader_module;
    vk::ShaderModule fragment_shader_module;
    std::vector<vk::PipelineShaderStageCreateInfo> shader_stages;

    texture_map staging_texture;
    std::vector<texture_map> atlas_textures;

    std::array<vk::DescriptorImageInfo, atlas_maximum_num_images> atlas_descriptor_image_infos;
    vk::Sampler atlas_sampler;
    vk::DescriptorImageInfo atlas_sampler_descriptor_image_info;

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

    

    /** Allocate pages from the atlas.
     */
    std::vector<page> allocate_pages(size_t num_pages) noexcept;

    /** Deallocate pages back to the atlas.
     */
    void free_pages(std::vector<page> const &pages) noexcept;

    /** Allocate an image in the atlas.
     * @param width The width of the image.
     * @param height The height of the image.
     * @return An image with allocated pages in the atlas.
     */
    image make_image(size_t width, size_t height) noexcept;

    void draw_in_command_buffer(vk::CommandBuffer &commandBuffer);

    /** Get the full staging pixel map excluding border.
     *
     * The returned pixel-map is offset by the page::border.
     */
    tt::pixel_map<sfloat_rgba16> get_staging_pixel_map();

    /** Prepare the atlas so that it can be used as a texture map by the shaders.
     */
    void prepare_atlas_for_rendering();

    /** Place vertices for a single image.
     *
     * @param vertices The list of vertices to add to.
     * @param clipping_rectangle The rectangle to clip the glyph.
     * @param box The rectangle of the image in window coordinates.
     * @param image The image to render.
     * @return True is atlas was updated.
     */
    bool place_vertices(
        vspan<vertex> &vertices,
        aarectangle const &clipping_rectangle,
        quad const &box,
        tt::pipeline_image::image const &image) noexcept;

private:
    std::vector<page> _atlas_free_pages;

    /** Get a submap of the stafing pixal map to draw the image in.
    */
    tt::pixel_map<sfloat_rgba16> get_staging_pixel_map(size_t width, size_t height)
    {
        return get_staging_pixel_map().submap(0, 0, width, height);
    }

    /** Copy the image from the staging pixel map into the atlas.
     */
    void update_atlas_with_staging_pixel_map(image const &image) noexcept;

    void build_shaders();
    void teardown_shaders(gfx_device_vulkan *vulkan_device);
    void add_atlas_image();
    void build_atlas();
    void teardown_atlas(gfx_device_vulkan *vulkan_device);

    friend image;
};

} // namespace tt::pipeline_image
