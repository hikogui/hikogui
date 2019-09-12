// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/PipelineImage_TextureMap.hpp"
#include "TTauri/GUI/PipelineImage_Page.hpp"
#include "TTauri/GUI/Device_forward.hpp"
#include "TTauri/Foundation/geometry.hpp"
#include "TTauri/Required/required.hpp"
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>
#include <mutex>

namespace TTauri::Draw {
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

    vk::Buffer indexBuffer;
    VmaAllocation indexBufferAllocation = {};

    vk::ShaderModule vertexShaderModule;
    vk::ShaderModule fragmentShaderModule;
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

    TextureMap stagingTexture;
    std::vector<TextureMap> atlasTextures;

    std::array<vk::DescriptorImageInfo, atlasMaximumNrImages> atlasDescriptorImageInfos;
    vk::Sampler atlasSampler;
    vk::DescriptorImageInfo atlasSamplerDescriptorImageInfo;

    std::vector<Page> atlasFreePages;
    std::unordered_map<std::string, std::weak_ptr<Image>> imageCache;

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

    /*! Get the coordinate in the atlast from a page index.
     * \param page number in the atlas
     * \return x, y pixel coordine in an atlasTexture and z the atlasTextureIndex.
     */
    static glm::ivec3 getAtlasPositionFromPage(Page page) noexcept {
        let imageIndex = page.nr / atlasNrPagesPerImage;
        let pageNrInsideImage = page.nr % atlasNrPagesPerImage;

        let pageY = pageNrInsideImage / atlasNrVerticalPages;
        let pageX = pageNrInsideImage % atlasNrVerticalPages;

        let x = pageX * Page::widthIncludingBorder + Page::border;
        let y = pageY * Page::heightIncludingBorder + Page::border;

        return {x, y, imageIndex};
    }

    std::vector<Page> getFreePages(int const nrPages);

    void returnPages(std::vector<Page> const &pages);

    /*! Get an image, possibly from the cache.
     * \param key of the image.
     * \param extent of the image.
     */
    std::shared_ptr<Image> getImage(std::string const &key, iextent2 extent);

    std::shared_ptr<Image> getImage(std::string const &key, extent2 extent) {
        return getImage(key, iextent2{extent.width(), extent.height()});
    }

    void drawInCommandBuffer(vk::CommandBuffer &commandBuffer);

    TTauri::Draw::PixelMap<uint32_t> getStagingPixelMap();

    void uploadPixmapToAtlas(Image const &image, Draw::PixelMap<wsRGBA> const &pixelMap);

    void prepareAtlasForRendering();

private:
    TTauri::Draw::PixelMap<uint32_t> getStagingPixelMap(iextent2 extent) {
        return getStagingPixelMap().submap({{0,0}, extent});
    }

    void updateAtlasWithStagingPixelMap(Image const &image);

    void buildIndexBuffer();
    void teardownIndexBuffer(gsl::not_null<Device_vulkan *> vulkanDevice);
    void buildShaders();
    void teardownShaders(gsl::not_null<Device_vulkan *> vulkanDevice);
    void addAtlasImage();
    void buildAtlas();
    void teardownAtlas(gsl::not_null<Device_vulkan *> vulkanDevice);
};

}
