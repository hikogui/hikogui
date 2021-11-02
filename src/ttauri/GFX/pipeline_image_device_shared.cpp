// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "pipeline_image.hpp"
#include "pipeline_image_device_shared.hpp"
#include "paged_image.hpp"
#include "gfx_device_vulkan.hpp"
#include "../pixel_map.hpp"
#include "../pixel_map.inl"
#include "../URL.hpp"
#include "../memory.hpp"
#include "../cast.hpp"
#include <array>

namespace tt::pipeline_image {

device_shared::device_shared(gfx_device_vulkan const &device) : device(device)
{
    build_shaders();
    build_atlas();
}

device_shared::~device_shared() {}

void device_shared::destroy(gfx_device_vulkan *vulkan_device)
{
    tt_axiom(vulkan_device);
    teardown_shaders(vulkan_device);
    teardown_atlas(vulkan_device);
}

std::vector<size_t> device_shared::allocate_pages(size_t num_pages) noexcept
{
    while (num_pages > _atlas_free_pages.size()) {
        add_atlas_image();
    }

    auto r = std::vector<size_t>();
    for (int i = 0; i < num_pages; i++) {
        ttlet page = _atlas_free_pages.back();
        r.push_back(page);
        _atlas_free_pages.pop_back();
    }
    return r;
}

void device_shared::free_pages(std::vector<size_t> const &pages) noexcept
{
    _atlas_free_pages.insert(_atlas_free_pages.end(), pages.begin(), pages.end());
}

tt::pixel_map<sfloat_rgba16> device_shared::get_staging_pixel_map()
{
    staging_texture.transitionLayout(device, vk::Format::eR16G16B16A16Sfloat, vk::ImageLayout::eGeneral);

    return staging_texture.pixel_map.submap(
        page_border, page_border, staging_image_width - 2 * page_border, staging_image_height - 2 * page_border);
}

/** Get the coordinate in the atlas from a page index.
 *
 * \param page number in the atlas
 * \return x, y pixel coordinate in an atlasTexture and z the atlasTextureIndex. Inside the border.
 */
static point3 get_atlas_position_from_page(size_t page) noexcept
{
    constexpr auto num_pages_per_image = device_shared::atlas_num_pages_per_image;
    constexpr auto num_vertical_pages = device_shared::atlas_num_pages_per_axis;
    constexpr auto page_border_size_border = f32x4{
        narrow_cast<float>(paged_image::page_border + paged_image::page_size + paged_image::page_border),
        narrow_cast<float>(paged_image::page_border + paged_image::page_size + paged_image::page_border),
        1.0f,
        1.0f};
    constexpr auto page_border =
        f32x4{narrow_cast<float>(paged_image::page_border), narrow_cast<float>(paged_image::page_border)};

    ttlet image_nr = narrow_cast<float>(page / num_pages_per_image);
    ttlet page_nr_inside_image = page % num_pages_per_image;

    ttlet page_xy = f32x4{i32x4{
        narrow_cast<int32_t>(page_nr_inside_image % num_vertical_pages),
        narrow_cast<int32_t>(page_nr_inside_image / num_vertical_pages),
        narrow_cast<int32_t>(image_nr),
        1}};

    return point3{page_xy * page_border_size_border + page_border};
}

/** Get the rectangle in the staging texture map to copy from.
 *
 * @param image The image
 * @param page_index The index of the page of the image.
 * @return The rectangle excluding the border into the staging map.
 */
static aarectangle get_staging_rectangle_from_page(const paged_image &image, size_t page_index)
{
    ttlet[pages_width, pages_height] = image.size_in_int_pages();
    ttlet left = (page_index % pages_width) * paged_image::page_size;
    ttlet bottom = (page_index / pages_height) * paged_image::page_size;
    ttlet right = std::min(left + paged_image::page_size, image.width);
    ttlet top = std::min(bottom + paged_image::page_size, image.height);

    ttlet p0 = point2{narrow_cast<float>(left), narrow_cast<float>(bottom)};
    ttlet p3 = point2{narrow_cast<float>(right), narrow_cast<float>(top)};
    return aarectangle{p0, p3};
}

void device_shared::update_atlas_with_staging_pixel_map(const paged_image &image) noexcept
{
    // Start with the actual image inside the stagingImage.
    auto rectangle = aarectangle{
        point2{narrow_cast<float>(page_border), narrow_cast<float>(page_border)},
        extent2{narrow_cast<float>(image.width), narrow_cast<float>(image.height)}};

    // Add one pixel of border around the actual image and keep extending
    // until the full border width is finished.
    for (int b = 0; b < page_border; b++) {
        rectangle = rectangle + 1.0f;

        auto pixel_map = staging_texture.pixel_map.submap(rectangle);
        makeTransparentBorder(pixel_map);
    }

    // Flush the given image, included the border.
    device.flushAllocation(
        staging_texture.allocation,
        0,
        ((page_border + image.width + page_border) * staging_texture.pixel_map.stride()) * sizeof(uint32_t));

    staging_texture.transitionLayout(device, vk::Format::eR16G16B16A16Sfloat, vk::ImageLayout::eTransferSrcOptimal);

    std::array<std::vector<vk::ImageCopy>, atlas_maximum_num_images> regions_to_copy_per_atlas_texture;
    for (size_t index = 0; index < std::size(image.pages); index++) {
        ttlet page = image.pages.at(index);

        ttlet border_offset = translate2{narrow_cast<float>(page_border), narrow_cast<float>(page_border)};
        ttlet src_rectangle = border_offset * get_staging_rectangle_from_page(image, index) + page_border;
        ttlet dst_position = ~border_offset * get_atlas_position_from_page(page);

        auto &regionsToCopy = regions_to_copy_per_atlas_texture.at(narrow_cast<size_t>(dst_position.z()));
        regionsToCopy.push_back(
            {{vk::ImageAspectFlagBits::eColor, 0, 0, 1},
             {narrow_cast<int32_t>(src_rectangle.left()), narrow_cast<int32_t>(src_rectangle.bottom()), 0},
             {vk::ImageAspectFlagBits::eColor, 0, 0, 1},
             {narrow_cast<int32_t>(dst_position.x()), narrow_cast<int32_t>(dst_position.y()), 0},
             {narrow_cast<uint32_t>(src_rectangle.width()), narrow_cast<uint32_t>(src_rectangle.height()), 1}});
    }

    for (size_t atlas_texture_index = 0; atlas_texture_index < size(atlas_textures); atlas_texture_index++) {
        ttlet &regions_to_copy = regions_to_copy_per_atlas_texture.at(atlas_texture_index);
        if (regions_to_copy.empty()) {
            continue;
        }

        auto &atlas_texture = atlas_textures.at(atlas_texture_index);
        atlas_texture.transitionLayout(device, vk::Format::eR16G16B16A16Sfloat, vk::ImageLayout::eTransferDstOptimal);

        device.copyImage(
            staging_texture.image,
            vk::ImageLayout::eTransferSrcOptimal,
            atlas_texture.image,
            vk::ImageLayout::eTransferDstOptimal,
            regions_to_copy);
    }
}

void device_shared::prepare_atlas_for_rendering()
{
    for (auto &atlas_texture : atlas_textures) {
        atlas_texture.transitionLayout(device, vk::Format::eR16G16B16A16Sfloat, vk::ImageLayout::eShaderReadOnlyOptimal);
    }
}

void device_shared::draw_in_command_buffer(vk::CommandBuffer &commandBuffer)
{
    commandBuffer.bindIndexBuffer(device.quadIndexBuffer, 0, vk::IndexType::eUint16);
}

void device_shared::build_shaders()
{
    vertex_shader_module = device.loadShader(URL("resource:GUI/pipeline_image.vert.spv"));
    fragment_shader_module = device.loadShader(URL("resource:GUI/pipeline_image.frag.spv"));

    shader_stages = {
        {vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, vertex_shader_module, "main"},
        {vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment, fragment_shader_module, "main"}};
}

void device_shared::teardown_shaders(gfx_device_vulkan *vulkanDevice)
{
    tt_axiom(vulkanDevice);
    vulkanDevice->destroy(vertex_shader_module);
    vulkanDevice->destroy(fragment_shader_module);
}

void device_shared::add_atlas_image()
{
    ttlet current_image_index = size(atlas_textures);

    // Create atlas image
    vk::ImageCreateInfo const imageCreateInfo = {
        vk::ImageCreateFlags(),
        vk::ImageType::e2D,
        vk::Format::eR16G16B16A16Sfloat,
        vk::Extent3D(atlas_image_axis_size, atlas_image_axis_size, 1),
        1, // mipLevels
        1, // arrayLayers
        vk::SampleCountFlagBits::e1,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        vk::SharingMode::eExclusive,
        0,
        nullptr,
        vk::ImageLayout::eUndefined};
    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    ttlet[atlasImage, atlasImageAllocation] = device.createImage(imageCreateInfo, allocationCreateInfo);

    ttlet atlasImageView = device.createImageView(
        {vk::ImageViewCreateFlags(),
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
         }});

    atlas_textures.push_back({atlasImage, atlasImageAllocation, atlasImageView});

    // Add pages for this image to free list.
    ttlet page_offset = current_image_index * atlas_num_pages_per_image;
    for (int i = 0; i < atlas_num_pages_per_image; i++) {
        _atlas_free_pages.push_back({page_offset + i});
    }

    // Build image descriptor info.
    for (size_t i = 0; i < size(atlas_descriptor_image_infos); i++) {
        // Point the descriptors to each imageView,
        // repeat the first imageView if there are not enough.
        atlas_descriptor_image_infos.at(i) = {
            vk::Sampler(),
            i < atlas_textures.size() ? atlas_textures.at(i).view : atlas_textures.at(0).view,
            vk::ImageLayout::eShaderReadOnlyOptimal};
    }
}

void device_shared::build_atlas()
{
    // Create staging image
    vk::ImageCreateInfo const imageCreateInfo = {
        vk::ImageCreateFlags(),
        vk::ImageType::e2D,
        vk::Format::eR16G16B16A16Sfloat,
        vk::Extent3D(staging_image_width, staging_image_height, 1),
        1, // mipLevels
        1, // arrayLayers
        vk::SampleCountFlagBits::e1,
        vk::ImageTiling::eLinear,
        vk::ImageUsageFlagBits::eTransferSrc,
        vk::SharingMode::eExclusive,
        0,
        nullptr,
        vk::ImageLayout::ePreinitialized};
    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    ttlet[image, allocation] = device.createImage(imageCreateInfo, allocationCreateInfo);
    ttlet data = device.mapMemory<sfloat_rgba16>(allocation);

    staging_texture = {
        image,
        allocation,
        vk::ImageView(),
        tt::pixel_map<sfloat_rgba16>{data.data(), imageCreateInfo.extent.width, imageCreateInfo.extent.height}};

    vk::SamplerCreateInfo const samplerCreateInfo = {
        vk::SamplerCreateFlags(),
        vk::Filter::eLinear, // magFilter
        vk::Filter::eLinear, // minFilter
        vk::SamplerMipmapMode::eNearest, // mipmapMode
        vk::SamplerAddressMode::eRepeat, // addressModeU
        vk::SamplerAddressMode::eRepeat, // addressModeV
        vk::SamplerAddressMode::eRepeat, // addressModeW
        0.0, // mipLodBias
        VK_FALSE, // anisotropyEnable
        0.0, // maxAnisotropy
        VK_FALSE, // compareEnable
        vk::CompareOp::eNever,
        0.0, // minLod
        0.0, // maxLod
        vk::BorderColor::eFloatTransparentBlack,
        VK_FALSE // unnormazlizedCoordinates
    };
    atlas_sampler = device.createSampler(samplerCreateInfo);

    atlas_sampler_descriptor_image_info = {atlas_sampler, vk::ImageView(), vk::ImageLayout::eUndefined};

    // There needs to be at least one atlas image, so the array of samplers can point to
    // the single image.
    add_atlas_image();
}

void device_shared::teardown_atlas(gfx_device_vulkan *vulkan_device)
{
    tt_axiom(vulkan_device);
    vulkan_device->destroy(atlas_sampler);

    for (const auto &atlas_texture : atlas_textures) {
        vulkan_device->destroy(atlas_texture.view);
        vulkan_device->destroyImage(atlas_texture.image, atlas_texture.allocation);
    }
    atlas_textures.clear();

    vulkan_device->unmapMemory(staging_texture.allocation);
    vulkan_device->destroyImage(staging_texture.image, staging_texture.allocation);
}

void device_shared::place_vertices(
    vspan<vertex> &vertices,
    aarectangle const &clipping_rectangle,
    quad const &box,
    paged_image const &image) noexcept
{
    tt_axiom(image.state == paged_image::state_type::uploaded);

    constexpr auto page_size2 = f32x4{i32x4{narrow_cast<int32_t>(page_size), narrow_cast<int32_t>(page_size)}};

    ttlet image_size = f32x4{i32x4{narrow_cast<int32_t>(image.width), narrow_cast<int32_t>(image.height)}};
    ttlet size_in_float_pages = f32x4{image.size_in_float_pages()};
    ttlet size_in_int_pages = i32x4{ceil(size_in_float_pages)};
    ttlet num_columns = narrow_cast<size_t>(size_in_int_pages.x());
    ttlet num_rows = narrow_cast<size_t>(size_in_int_pages.y());

    ttlet page_to_quad_ratio = rcp(size_in_float_pages);
    ttlet page_to_quad_ratio_x = scale3{page_to_quad_ratio.xxx1()};
    ttlet page_to_quad_ratio_y = scale3{page_to_quad_ratio.yyy1()};
    ttlet left_increment = page_to_quad_ratio_y * box.left();
    ttlet right_increment = page_to_quad_ratio_y * box.right();

    auto left_bottom = box.p0;
    auto right_bottom = box.p1;
    auto bottom_increment = page_to_quad_ratio_x * (right_bottom - left_bottom);
    auto it = image.pages.begin();
    for (size_t page_index = 0, row_nr = 0; row_nr != num_rows; ++row_nr) {
        ttlet left_top = left_bottom + left_increment;
        ttlet right_top = right_bottom + right_increment;
        ttlet top_increment = page_to_quad_ratio_x * (right_top - left_top);

        auto new_p0 = left_bottom;
        auto new_p2 = left_top;
        for (size_t column_nr = 0; column_nr != num_columns; ++column_nr, ++page_index, ++it) {
            ttlet new_p1 = new_p0 + bottom_increment;
            ttlet new_p3 = new_p2 + top_increment;

            // The new quad, limited to the right-top corner of the original quad.
            ttlet polygon = quad{new_p0, min(new_p1, box.p3), min(new_p2, box.p3), min(new_p3, box.p3)};
            ttlet atlas_position = get_atlas_position_from_page(*it);

            static_assert(std::popcount(page_size) == 1);
            constexpr int page_size_shift = std::countr_zero(page_size);
            ttlet xy = f32x4{i32x4{narrow_cast<int32_t>(column_nr), narrow_cast<int32_t>(row_nr)} << page_size_shift};
            ttlet uv_rectangle = rectangle{atlas_position, extent2{min(image_size - xy, page_size2)}};

            vertices.emplace_back(polygon.p0, clipping_rectangle, get<0>(uv_rectangle));
            vertices.emplace_back(polygon.p1, clipping_rectangle, get<1>(uv_rectangle));
            vertices.emplace_back(polygon.p2, clipping_rectangle, get<2>(uv_rectangle));
            vertices.emplace_back(polygon.p3, clipping_rectangle, get<3>(uv_rectangle));

            new_p0 = new_p1;
            new_p2 = new_p3;
        }

        left_bottom = left_top;
        right_bottom = right_top;
        bottom_increment = top_increment;
    }
}

} // namespace tt::pipeline_image
