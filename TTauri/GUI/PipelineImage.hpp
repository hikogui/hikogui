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
#include "TTauri/geometry.hpp"

#include <vma/vk_mem_alloc.h>
#include <gsl/gsl>

namespace TTauri::GUI {

class Device_vulkan;

/*! Pipeline for rendering backings of widgets.
 * Maintains texture map atlasses and sharing for all views.
 */
class PipelineImage : public Pipeline_vulkan {
public:
    static const size_t maximumNumberOfVertices = 65536;
    static const size_t maximumNumberOfSquares = maximumNumberOfVertices / 4;
    static const size_t maximumNumberOfTriangles = maximumNumberOfSquares * 2;
    static const size_t maximumNumberOfIndices = maximumNumberOfTriangles * 3;

    struct DeviceShared;
    struct Image;

    struct PushConstants {
        glm::vec2 windowExtent = { 0.0, 0.0 };
        glm::vec2 viewportScale = { 0.0, 0.0 };
        glm::vec2 atlasExtent = { 0.0, 0.0 };
        glm::vec2 atlasScale = { 0.0, 0.0 };

        static std::vector<vk::PushConstantRange> pushConstantRanges()
        {
            return {
                { vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(PushConstants) }
            };
        }
    };

    /*! Information on the location and orientation of an image on a window.
     */
    struct ImageLocation {
        //! The pixel-coordinates where the origin is located relative to the top-left corner of the window.
        glm::vec2 position;

        //! Location of the origin relative to the top-left of the image in number of pixels.
        glm::vec2 origin;

        //! Clockwise rotation around the origin of the image in radials.
        float rotation;

        //! The position in pixels of the clipping rectangle relative to the top-left corner of the window, and extent in pixels.
        u16rect clippingRectangle;

        //! Depth location of the rendered image.
        uint16_t depth;

        //! Transparency of the image.
        uint8_t alpha;
    };

    /*! A vertex defining a rectangle on a window.
     * The vertex shader will convert window pixel-coordinates to normalized projection-coordinates.
     */
    struct Vertex {
        //! The pixel-coordinates where the origin is located relative to the top-left corner of the window.
        glm::vec2 position;

        //! The position in pixels of the clipping rectangle relative to the top-left corner of the window, and extent in pixels.
        u16rect clippingRectangle;

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

    class Delegate {
    public:
        struct Error : virtual boost::exception, virtual std::exception {};

        virtual void pipelineImagePlaceVertices(gsl::span<Vertex> &vertices, size_t &offset) = 0;
    };

    PipelineImage(const std::shared_ptr<Window> window);
    ~PipelineImage() {};

    PipelineImage(const PipelineImage &) = delete;
    PipelineImage &operator=(const PipelineImage &) = delete;
    PipelineImage(PipelineImage &&) = delete;
    PipelineImage &operator=(PipelineImage &&) = delete;

    vk::Semaphore render(uint32_t imageIndex, vk::Semaphore inputSemaphore) override;

protected:
    PushConstants pushConstants;
    size_t numberOfAtlasImagesInDescriptor = 0;

    size_t numberOfVertices = 0;
    std::vector<vk::Buffer> vertexBuffers;
    std::vector<VmaAllocation> vertexBuffersAllocation;
    std::vector<gsl::span<Vertex>> vertexBuffersData;

    void drawInCommandBuffer(vk::CommandBuffer &commandBuffer, uint32_t imageIndex) override;

    std::vector<vk::PipelineShaderStageCreateInfo> createShaderStages() const override;
    std::vector<vk::DescriptorSetLayoutBinding> createDescriptorSetLayoutBindings() const override;
    std::vector<vk::WriteDescriptorSet> createWriteDescriptorSet(uint32_t imageIndex) const override;
    virtual uint64_t getDescriptorSetVersion() const override;
    std::vector<vk::PushConstantRange> createPushConstantRanges() const override { return PushConstants::pushConstantRanges(); }
    vk::VertexInputBindingDescription createVertexInputBindingDescription() const override { return Vertex::inputBindingDescription(); }
    std::vector<vk::VertexInputAttributeDescription> createVertexInputAttributeDescriptions() const override { return Vertex::inputAttributeDescriptions(); }

private:
    void buildVertexBuffers(size_t nrFrameBuffers) override;
    void teardownVertexBuffers() override;
};

}