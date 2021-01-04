// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "PipelineImage_TextureMap.hpp"
#include "PipelineImage_Page.hpp"
#include "../required.hpp"
#include "../R16G16B16A16SFloat.hpp"
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>
#include <mutex>

namespace tt {
class gui_device_vulkan;
template<typename T> class pixel_map;
}

namespace tt::PipelineImage {

struct Image;

struct DeviceShared final {
    static constexpr int atlasNrHorizontalPages = 16;
    static constexpr int atlasNrVerticalPages = 16;
    static constexpr int atlasImageWidth = atlasNrHorizontalPages * Page::widthIncludingBorder;
    static constexpr int atlasImageHeight = atlasNrVerticalPages * Page::heightIncludingBorder;
    static constexpr int atlasNrPagesPerImage = atlasNrHorizontalPages * atlasNrVerticalPages;
    static constexpr int atlasMaximumNrImages = 16;
    static constexpr int stagingImageWidth = 1024;
    static constexpr int stagingImageHeight = 1024;

    gui_device_vulkan const &device;

    vk::ShaderModule vertexShaderModule;
    vk::ShaderModule fragmentShaderModule;
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

    TextureMap stagingTexture;
    std::vector<TextureMap> atlasTextures;

    std::array<vk::DescriptorImageInfo, atlasMaximumNrImages> atlasDescriptorImageInfos;
    vk::Sampler atlasSampler;
    vk::DescriptorImageInfo atlasSamplerDescriptorImageInfo;

    std::vector<Page> atlasFreePages;

    DeviceShared(gui_device_vulkan const &device);
    ~DeviceShared();

    DeviceShared(DeviceShared const &) = delete;
    DeviceShared &operator=(DeviceShared const &) = delete;
    DeviceShared(DeviceShared &&) = delete;
    DeviceShared &operator=(DeviceShared &&) = delete;

    /*! Deallocate vulkan resources.
    * This is called in the destructor of gui_device_vulkan, therefor we can not use our `std::weak_ptr<gui_device_vulkan> device`.
    */
    void destroy(gui_device_vulkan *vulkanDevice);

    /*! Get the coordinate in the atlas from a page index.
     * \param page number in the atlas
     * \return x, y pixel coordinate in an atlasTexture and z the atlasTextureIndex.
     */
    static i32x4 getAtlasPositionFromPage(Page page) noexcept {
        ttlet imageIndex = page.nr / atlasNrPagesPerImage;
        ttlet pageNrInsideImage = page.nr % atlasNrPagesPerImage;

        ttlet pageY = pageNrInsideImage / atlasNrVerticalPages;
        ttlet pageX = pageNrInsideImage % atlasNrVerticalPages;

        ttlet x = pageX * Page::widthIncludingBorder + Page::border;
        ttlet y = pageY * Page::heightIncludingBorder + Page::border;

        return i32x4{narrow_cast<int>(x), narrow_cast<int>(y), narrow_cast<int>(imageIndex), 1};
    }

    /** Allocate pages from the atlas.
     */
    std::vector<Page> allocatePages(int const nrPages) noexcept;

    /** Deallocate pages back to the atlas.
     */
    void freePages(std::vector<Page> const &pages) noexcept;

    /** Allocate an image in the atlas.
     * \param extent of the image.
     */
    Image makeImage(i32x4 extent) noexcept;

    void drawInCommandBuffer(vk::CommandBuffer &commandBuffer);

    tt::pixel_map<R16G16B16A16SFloat> getStagingPixelMap();

    void prepareAtlasForRendering();

private:
    tt::pixel_map<R16G16B16A16SFloat> getStagingPixelMap(i32x4 extent) {
        return getStagingPixelMap().submap({i32x4::point(0,0), extent});
    }

    void updateAtlasWithStagingPixelMap(Image const &image);

    void buildShaders();
    void teardownShaders(gui_device_vulkan *vulkanDevice);
    void addAtlasImage();
    void buildAtlas();
    void teardownAtlas(gui_device_vulkan *vulkanDevice);

    friend Image;
};

}
