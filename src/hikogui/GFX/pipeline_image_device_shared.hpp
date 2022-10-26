// Copyright Take Vos 2019, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "pipeline_image_texture_map.hpp"
#include "pipeline_image_vertex.hpp"
#include "paged_image.hpp"
#include "../utility.hpp"
#include "../rapid/sfloat_rgba16.hpp"
#include "../geometry/quad.hpp"
#include "../vector_span.hpp"
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>
#include <mutex>

namespace hi::inline v1 {
class gfx_device_vulkan;
template<typename T>
class pixel_map;

namespace pipeline_image {

struct device_shared {
    static constexpr std::size_t atlas_num_pages_per_axis = 8;
    static constexpr std::size_t atlas_num_pages_per_image = atlas_num_pages_per_axis * atlas_num_pages_per_axis;
    static constexpr std::size_t atlas_image_axis_size = atlas_num_pages_per_axis * (paged_image::page_size + 2);
    static constexpr std::size_t atlas_maximum_num_images = 64;
    static constexpr std::size_t staging_image_width = 1024;
    static constexpr std::size_t staging_image_height = 1024;

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
    std::vector<std::size_t> allocate_pages(std::size_t num_pages) noexcept;

    /** Deallocate pages back to the atlas.
     */
    void free_pages(std::vector<std::size_t> const &pages) noexcept;

    void draw_in_command_buffer(vk::CommandBuffer &commandBuffer);

    /** Get the full staging pixel map excluding border.
     *
     * The returned pixel-map is offset by the page::border.
     */
    hi::pixel_map<sfloat_rgba16> get_staging_pixel_map();

    /** Prepare the atlas so that it can be used as a texture map by the shaders.
     */
    void prepare_atlas_for_rendering();

    /** Place vertices for a single image.
     *
     * @pre The image is uploaded.
     * @param vertices The list of vertices to add to.
     * @param clipping_rectangle The rectangle to clip the glyph.
     * @param box The rectangle of the image in window coordinates.
     * @param image The image to render.
     */
    void place_vertices(
        vector_span<vertex> &vertices,
        aarectangle const &clipping_rectangle,
        quad const &box,
        paged_image const &image) noexcept;

private:
    std::vector<std::size_t> _atlas_free_pages;

    /** Get a submap of the staging pixel map to draw the image in.
     */
    hi::pixel_map<sfloat_rgba16> get_staging_pixel_map(std::size_t width, std::size_t height)
    {
        return get_staging_pixel_map().submap(0, 0, width, height);
    }

    /** Add a transparent border around the image.
     *
     * @param border_rectangle The rectangle of the border, the image-rectangle is inside this 1 pixel border.
     */
    void make_staging_border_transparent(aarectangle border_rectangle) noexcept;

    /** Clear the area between the border rectangle and upload rectangle.
    * 
    * @param border_rectangle The rectangle where the border is located.
    * @param upload_rectangle The rectangle which will be uploaded to the atlas.
     */
    void clear_staging_between_border_and_upload(aarectangle border_rectangle, aarectangle upload_rectangle) noexcept;

    /** Prepare the staging image for upload.
     *
     * The following will be done.
     *  * Around the edge of the image the color is copied into the 1 pixel border.
     *    with the alpha channel set to zero.
     *  * On the right and upper edge the pixels are set to transparent-black up to
     *    a multiple of the `paged_image::page_size`.
     *  * flush the image to the GPU
     *  * transition the image for transferring to the atlas.
     */
    void prepare_staging_for_upload(paged_image const &image) noexcept;

    /** Copy the image from the staging pixel map into the atlas.
     */
    void update_atlas_with_staging_pixel_map(paged_image const &image) noexcept;

    void build_shaders();
    void teardown_shaders(gfx_device_vulkan *vulkan_device);
    void add_atlas_image();
    void build_atlas();
    void teardown_atlas(gfx_device_vulkan *vulkan_device);

    friend paged_image;
};

} // namespace pipeline_image
} // namespace hi::inline v1
