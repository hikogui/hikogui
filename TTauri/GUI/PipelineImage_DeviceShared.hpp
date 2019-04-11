
#pragma once

#include "PipelineImage.hpp"
#include "Device_vulkan.hpp"

#include "TTauri/Draw/PixelMap.hpp"

#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

namespace TTauri::GUI {

struct PipelineImage::DeviceShared final {
    struct Error : virtual boost::exception, virtual std::exception {};

    static const size_t atlasImageWidth = 4096;
    static const size_t atlasImageHeight = 4096;
    static const size_t stagingImageWidth = 2048;
    static const size_t stagingImageHeight = 1024;
    static const size_t atlasSliceWidth = 64;
    static const size_t atlasSliceHeight = 64;
    static const size_t atlasNrHorizontalSlices = atlasImageWidth / atlasSliceWidth;
    static const size_t atlasNrVerticalSlices = atlasImageHeight / atlasSliceHeight;
    static const size_t atlasNrSlicesPerImage = atlasNrHorizontalSlices * atlasNrVerticalSlices;
    static const size_t atlasMaximumNrImages = 65536 / atlasNrSlicesPerImage;

    std::weak_ptr<Device_vulkan> device;

    vk::Buffer indexBuffer;
    VmaAllocation indexBufferAllocation = {};

    vk::ShaderModule vertexShaderModule;
    vk::ShaderModule fragmentShaderModule;
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

    struct TextureMap {
        vk::Image image;
        VmaAllocation allocation = {};
        vk::ImageView view;
        TTauri::Draw::PixelMap<uint32_t> pixelMap;
        vk::ImageLayout layout = vk::ImageLayout::eUndefined;

        void transitionLayout(const Device_vulkan &device, vk::Format format, vk::ImageLayout nextLayout) {
            if (layout != nextLayout) {
                device.transitionLayout(image, format, layout, nextLayout);
                layout = nextLayout;
            }
        }
    };
    TextureMap stagingTexture;
    std::vector<TextureMap> atlasTextures;

    std::vector<uint16_t> atlasFreeSlices;
    std::array<vk::DescriptorImageInfo, atlasMaximumNrImages> atlasDescriptorImageInfos;
    vk::Sampler atlasSampler;
    vk::DescriptorImageInfo atlasSamplerDescriptorImageInfo;

    std::unordered_map<std::string, std::shared_ptr<PipelineImage::Image>> viewImages;

    DeviceShared(const std::shared_ptr<Device_vulkan> device);
    ~DeviceShared();

    DeviceShared(const DeviceShared &) = delete;
    DeviceShared &operator=(const DeviceShared &) = delete;
    DeviceShared(DeviceShared &&) = delete;
    DeviceShared &operator=(DeviceShared &&) = delete;

    /*! Deallocate vulkan resources.
    * This is called in the destructor of Device_vulkan, therefor we can not use our `std::weak_ptr<Device_vulkan> device`.
    */
    void destroy(gsl::not_null<Device_vulkan *> vulkanDevice);

    /*! Get the coordinate in the atlast from a slice index.
     */
    static u16vec3 getAtlasPositionFromSlice(uint16_t slice) {
        uint16_t const imageIndex = slice / atlasNrSlicesPerImage;
        slice %= atlasNrSlicesPerImage;

        uint16_t const y = slice / atlasNrVerticalSlices;
        slice %= atlasNrVerticalSlices;

        uint16_t const x = slice;

        return {x, y, imageIndex};
    }

    std::vector<uint16_t> getFreeSlices(size_t const nrSlices);

    std::shared_ptr<PipelineImage::Image> retainImage(const std::string &key, u16vec2 extent);
    void releaseImage(const std::shared_ptr<PipelineImage::Image> &image);

    /*! Exchange an image when the key is different.
     * \param image A shared pointer to an image, which may be reseated.
     * \param key of the image.
     * \param extent of the image.
     */
    void exchangeImage(std::shared_ptr<PipelineImage::Image> &image, const std::string &key, const u16vec2 extent);

    void drawInCommandBuffer(vk::CommandBuffer &commandBuffer);

    TTauri::Draw::PixelMap<uint32_t> getStagingPixelMap();
    void updateAtlasWithStagingPixelMap(const PipelineImage::Image &image);
    void prepareAtlasForRendering();

private:
    void buildIndexBuffer();
    void teardownIndexBuffer(gsl::not_null<Device_vulkan *> vulkanDevice);
    void buildShaders();
    void teardownShaders(gsl::not_null<Device_vulkan *> vulkanDevice);
    void addAtlasImage();
    void buildAtlas();
    void teardownAtlas(gsl::not_null<Device_vulkan *> vulkanDevice);
};

}