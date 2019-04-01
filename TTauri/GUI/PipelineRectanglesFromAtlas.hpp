//
//  BackingPipeline.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-12.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include "Pipeline_vulkan.hpp"
#include "config.hpp"
#include "geometry.hpp"

#include <vma/vk_mem_alloc.h>
#include <gsl/gsl>

namespace TTauri::GUI {

class Device_vulkan;

/*! Pipeline for rendering backings of widgets.
 * Maintains texture map atlasses and sharing for all views.
 */
class PipelineRectanglesFromAtlas : public Pipeline_vulkan {
public:
    static const size_t maximumNumberOfVertices = 65536;
    static const size_t maximumNumberOfSquares = maximumNumberOfVertices / 4;
    static const size_t maximumNumberOfTriangles = maximumNumberOfSquares * 2;
    static const size_t maximumNumberOfIndices = maximumNumberOfTriangles * 3;

    struct DeviceShared final {
        std::weak_ptr<Device_vulkan> device;

        vk::Buffer indexBuffer;
        VmaAllocation indexBufferAllocation = {};

        DeviceShared(const std::shared_ptr<Device_vulkan> device);
        ~DeviceShared();

        /*! Deallocate vulkan resources.
         * This is called in the destructor of Device_vulkan, therefor we can not use our device weak_ptr.
         */
        void destroy(gsl::not_null<Device_vulkan *> vulkanDevice);

    private:
        void buildIndexBuffer();
        void teardownIndexBuffer(gsl::not_null<Device_vulkan *> vulkanDevice);
    };

    struct PushConstants {
        glm::vec2 windowExtent = { 0.0, 0.0 };
        glm::vec2 viewportScale = { 0.0, 0.0 };

        static std::vector<vk::PushConstantRange> pushConstantRanges()
        {
            return {
                { vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(PushConstants) }
            };
        }
    };

    /*! A vertex defining a rectangle on a window.
     * The same vertex is passed to the vertex shader 6 times for each rectangle (two triangles).
     * The vertex shader will convert window pixel-coordinates to normalized projection-coordinates.
     */
    struct Vertex {
        //! The pixel-coordinates where the origin is located relative to the top-left corner of the window.
        glm::vec2 position;

        //! The left-top and right-bottom position in pixels of the clipping rectangle relative to the top-left corner of the window.
        u16rect2 clippingRectangle;

        //! The x, y coord inside the texture-atlas, z is used as an index in the texture-atlas array
        u16vec3 atlasPosition;

        //! The depth for depth test.
        uint16_t depth;

        //! transparency of the image.
        uint8_t alpha;

        //! Align to 32 bits.
        uint8_t dummy[3];
     

        static vk::VertexInputBindingDescription inputBindingDescription()
        {
            return {
                0, sizeof(Vertex), vk::VertexInputRate::eVertex
            };
        }

        static std::vector<vk::VertexInputAttributeDescription> inputAttributeDescriptions()
        {
            return {
                { 0, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, position) },
                { 1, 0, vk::Format::eR16G16Uint, offsetof(Vertex, clippingRectangle.offset) },
                { 2, 0, vk::Format::eR16G16Uint, offsetof(Vertex, clippingRectangle.extent) },
                { 3, 0, vk::Format::eR16G16B16Uint, offsetof(Vertex, atlasPosition) },                
                { 4, 0, vk::Format::eR16Uint, offsetof(Vertex, depth) },
                { 5, 0, vk::Format::eR8Uint, offsetof(Vertex, alpha) },
            };
        }
    };

    struct Rectangle {
        std::string key;
        std::vector<uint16_t> atlasIndices;

        glm::vec2 origin;
        glm::vec2 position;
        float rotation;
        float alpha;
        u16vec2 extent;

        void placeVertices(gsl::span<Vertex> &vertices, size_t &offset);
    };

    class Delegate {
    public:
        struct Error : virtual boost::exception, virtual std::exception {};

        virtual size_t piplineRectangledFromAtlasPlaceVertices(const gsl::span<Vertex> &vertices, size_t offset) = 0;
    };

    PipelineRectanglesFromAtlas(const std::shared_ptr<Window> window);
    ~PipelineRectanglesFromAtlas() {};

    PipelineRectanglesFromAtlas(const PipelineRectanglesFromAtlas &) = delete;
    PipelineRectanglesFromAtlas &operator=(const PipelineRectanglesFromAtlas &) = delete;
    PipelineRectanglesFromAtlas(PipelineRectanglesFromAtlas &&) = delete;
    PipelineRectanglesFromAtlas &operator=(PipelineRectanglesFromAtlas &&) = delete;

    vk::Semaphore render(uint32_t imageIndex, vk::Semaphore inputSemaphore) override;

protected:
    PushConstants pushConstants;

    size_t numberOfVertices = 0;
    std::vector<vk::Buffer> vertexBuffers;
    std::vector<VmaAllocation> vertexBuffersAllocation;
    std::vector<gsl::span<Vertex>> vertexBuffersData;

    void drawInCommandBuffer(vk::CommandBuffer &commandBuffer, uint32_t imageIndex) override;
    std::vector<vk::ShaderModule> createShaderModules() const override;
    std::vector<vk::PipelineShaderStageCreateInfo> createShaderStages(const std::vector<vk::ShaderModule> &shaders) const override;
    std::vector<vk::PushConstantRange> createPushConstantRanges() const override;
    vk::VertexInputBindingDescription createVertexInputBindingDescription() const override;
    std::vector<vk::VertexInputAttributeDescription> createVertexInputAttributeDescriptions() const override;

    void buildVertexBuffers(size_t nrFrameBuffers) override;
    void teardownVertexBuffers() override;


};

}