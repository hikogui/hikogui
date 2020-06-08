// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/PipelineImage_TextureMap.hpp"
#include "TTauri/GUI/PipelineImage_Page.hpp"
#include "TTauri/GUI/Device_forward.hpp"
#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/R16G16B16A16SFloat.hpp"
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>
#include <mutex>

namespace TTauri {
template<typename T> struct PixelMap;
}

namespace TTauri::GUI::PipelineImage {

struct Image;

struct DeviceShared final {
    static constexpr int atlasNrHorizontalPages = 60;
    static constexpr int atlasNrVerticalPages = 60;
    static constexpr int atlasImageWidth = atlasNrHorizontalPages * Page::widthIncludingBorder;
    static constexpr int atlasImageHeight = atlasNrVerticalPages * Page::heightIncludingBorder;
    static constexpr int atlasNrPagesPerImage = atlasNrHorizontalPages * atlasNrVerticalPages;
    static constexpr int atlasMaximumNrImages = 16;
    static constexpr int stagingImageWidth = 2048;
    static constexpr int stagingImageHeight = 1024;

    Device const &device;

    vk::ShaderModule vertexShaderModule;
    vk::ShaderModule fragmentShaderModule;
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

    TextureMap stagingTexture;
    std::vector<TextureMap> atlasTextures;

    std::array<vk::DescriptorImageInfo, atlasMaximumNrImages> atlasDescriptorImageInfos;
    vk::Sampler atlasSampler;
    vk::DescriptorImageInfo atlasSamplerDescriptorImageInfo;

    std::vector<Page> atlasFreePages;

    DeviceShared(Device const &device);
    ~DeviceShared();

    DeviceShared(DeviceShared const &) = delete;
    DeviceShared &operator=(DeviceShared const &) = delete;
    DeviceShared(DeviceShared &&) = delete;
    DeviceShared &operator=(DeviceShared &&) = delete;

    /*! Deallocate vulkan resources.
    * This is called in the destructor of Device_vulkan, therefor we can not use our `std::weak_ptr<Device_vulkan> device`.
    */
    void destroy(Device *vulkanDevice);

    /*! Get the coordinate in the atlas from a page index.
     * \param page number in the atlas
     * \return x, y pixel coordinate in an atlasTexture and z the atlasTextureIndex.
     */
    static ivec getAtlasPositionFromPage(Page page) noexcept {
        let imageIndex = page.nr / atlasNrPagesPerImage;
        let pageNrInsideImage = page.nr % atlasNrPagesPerImage;

        let pageY = pageNrInsideImage / atlasNrVerticalPages;
        let pageX = pageNrInsideImage % atlasNrVerticalPages;

        let x = pageX * Page::widthIncludingBorder + Page::border;
        let y = pageY * Page::heightIncludingBorder + Page::border;

        return ivec{x, y, imageIndex, 1};
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
    Image makeImage(ivec extent) noexcept;

    void drawInCommandBuffer(vk::CommandBuffer &commandBuffer);

    TTauri::PixelMap<R16G16B16A16SFloat> getStagingPixelMap();

    void prepareAtlasForRendering();

private:
    TTauri::PixelMap<R16G16B16A16SFloat> getStagingPixelMap(ivec extent) {
        return getStagingPixelMap().submap({{0,0}, extent});
    }

    void updateAtlasWithStagingPixelMap(Image const &image);

    void buildShaders();
    void teardownShaders(Device_vulkan *vulkanDevice);
    void addAtlasImage();
    void buildAtlas();
    void teardownAtlas(Device_vulkan *vulkanDevice);

    friend Image;
};

}
