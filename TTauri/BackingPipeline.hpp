//
//  BackingPipeline.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-12.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include "Pipeline.hpp"
#include "config.hpp"

namespace TTauri {
namespace GUI {

/*! Pipeline for rendering backings of widgets.
 * Maintains texture map atlasses and sharing for all views.
 */
class BackingPipeline : public Pipeline {
public:
    struct PushConstants {
        glm::vec2 windowExtent;
        glm::vec2 viewportScale;

        static std::vector<vk::PushConstantRange> pushConstantRanges()
        {
            return {
                {vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(PushConstants)}
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
                {0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, position)},
                {1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, atlasPosition)},
                {2, 0, vk::Format::eR32Sfloat, offsetof(Vertex, atlasPosition)}
            };
        }
    };

    class Delegate {
    public:
        struct Error : virtual boost::exception, virtual std::exception {};

        virtual size_t BackingPipelineRender(Vertex *vertices, size_t offset, size_t size) = 0;
    };


    BackingPipeline(Window *window);
    virtual ~BackingPipeline();
    
    virtual vk::Semaphore render(uint32_t imageIndex, vk::Semaphore inputSemaphore);

protected:
    PushConstants pushConstants;
    size_t numberOfVertices;

    virtual void drawInCommandBuffer(vk::CommandBuffer &commandBuffer);
    virtual std::vector<vk::ShaderModule> createShaderModules() const;
    virtual std::vector<vk::PipelineShaderStageCreateInfo> createShaderStages(const std::vector<vk::ShaderModule> &shaders) const;
    virtual std::vector<vk::PushConstantRange> createPushConstantRanges() const;
    virtual vk::VertexInputBindingDescription createVertexInputBindingDescription() const;
    virtual std::vector<vk::VertexInputAttributeDescription> createVertexInputAttributeDescriptions() const;
    virtual size_t maximumNumberOfVertices() const { return backingPipelineMaximumNumberOfVertices; }

};

}}
