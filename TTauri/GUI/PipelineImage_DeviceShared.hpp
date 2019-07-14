// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "PipelineImage_TextureMap.hpp"
#include "PipelineImage_Page.hpp"
#include "Device_forward.hpp"
#include "TTauri/geometry.hpp"
#include "TTauri/required.hpp"
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>
#include <boost/exception/exception.hpp>
#include <mutex>

namespace TTauri::Draw {
template<typename T> struct PixelMap;
}

namespace TTauri::GUI::PipelineImage {

struct Image;

struct DeviceShared final {
    struct Error : virtual boost::exception, virtual std::exception {};

    static constexpr size_t atlasNrHorizontalPages = 60;
    static constexpr size_t atlasNrVerticalPages = 60;
    static constexpr size_t atlasImageWidth = atlasNrHorizontalPages * Page::widthIncludingBorder;
    static constexpr size_t atlasImageHeight = atlasNrVerticalPages * Page::heightIncludingBorder;
    static constexpr size_t atlasNrPagesPerImage = atlasNrHorizontalPages * atlasNrVerticalPages;
    static constexpr size_t atlasMaximumNrImages = 16;
    static constexpr size_t stagingImageWidth = 2048;
    static constexpr size_t stagingImageHeight = 1024;

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
    static glm::u64vec3 getAtlasPositionFromPage(Page page) {
        let imageIndex = page.nr / atlasNrPagesPerImage;
        let pageNrInsideImage = page.nr % atlasNrPagesPerImage;

        let pageY = pageNrInsideImage / atlasNrVerticalPages;
        let pageX = pageNrInsideImage % atlasNrVerticalPages;

        let x = pageX * Page::widthIncludingBorder + Page::border;
        let y = pageY * Page::heightIncludingBorder + Page::border;

        return {x, y, imageIndex};
    }

    std::vector<Page> getFreePages(size_t const nrPages);

    void returnPages(std::vector<Page> const &pages);

    /*! Get an image, possibly from the cache.
     * \param key of the image.
     * \param extent of the image.
     */
    std::shared_ptr<Image> getImage(std::string const &key, u64extent2 extent);

    std::shared_ptr<Image> getImage(std::string const &key, extent2 extent) {
        return getImage(key, u64extent2{extent.width(), extent.height()});
    }

    void drawInCommandBuffer(vk::CommandBuffer &commandBuffer);

    TTauri::Draw::PixelMap<uint32_t> getStagingPixelMap();

    void uploadPixmapToAtlas(Image const &image, Draw::PixelMap<wsRGBA> const &pixelMap);

    void prepareAtlasForRendering();

private:
    TTauri::Draw::PixelMap<uint32_t> getStagingPixelMap(u64extent2 extent) {
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
