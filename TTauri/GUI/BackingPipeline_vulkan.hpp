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

namespace TTauri {
namespace GUI {

/*! Pipeline for rendering backings of widgets.
 * Maintains texture map atlasses and sharing for all views.
 */
class BackingPipeline_vulkan : public Pipeline_vulkan {
public:
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

    struct Vertex {
        //! x, y position in window coordinate. z is depth for layering.
        glm::vec3 position;

        //! x, y position in the atlast coordinate. z selects one of the atlas images.
        glm::vec3 atlasPosition;

        //! transparency of the image.
        float alpha;

        static vk::VertexInputBindingDescription inputBindingDescription()
        {
            return {
                0, sizeof(Vertex), vk::VertexInputRate::eVertex
            };
        }

        static std::vector<vk::VertexInputAttributeDescription> inputAttributeDescriptions()
        {
            return {
                { 0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, position) },
                { 1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, atlasPosition) },
                { 2, 0, vk::Format::eR32Sfloat, offsetof(Vertex, atlasPosition) }
            };
        }
    };

    class Delegate {
    public:
        struct Error : virtual boost::exception, virtual std::exception {};

        virtual size_t backingPipelineRender(Vertex *vertices, size_t offset, size_t size) = 0;
    };

    BackingPipeline_vulkan(const std::shared_ptr<Window> &window);
    ~BackingPipeline_vulkan() {};

    BackingPipeline_vulkan(const BackingPipeline_vulkan &) = delete;
    BackingPipeline_vulkan &operator=(const BackingPipeline_vulkan &) = delete;
    BackingPipeline_vulkan(BackingPipeline_vulkan &&) = delete;
    BackingPipeline_vulkan &operator=(BackingPipeline_vulkan &&) = delete;

    vk::Semaphore render(uint32_t imageIndex, vk::Semaphore inputSemaphore) override;

protected:
    PushConstants pushConstants;
    size_t numberOfVertices = 0;

    void drawInCommandBuffer(vk::CommandBuffer &commandBuffer) override;
    std::vector<vk::ShaderModule> createShaderModules() const override;
    std::vector<vk::PipelineShaderStageCreateInfo> createShaderStages(const std::vector<vk::ShaderModule> &shaders) const override;
    std::vector<vk::PushConstantRange> createPushConstantRanges() const override;
    vk::VertexInputBindingDescription createVertexInputBindingDescription() const override;
    std::vector<vk::VertexInputAttributeDescription> createVertexInputAttributeDescriptions() const override;
    size_t maximumNumberOfVertices() const override { return backingPipelineMaximumNumberOfVertices; }
};

}}
