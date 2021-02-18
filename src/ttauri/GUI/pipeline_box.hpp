// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "pipeline_vulkan.hpp"
#include "pipeline_box_push_constants.hpp"
#include "pipeline_box_vertex.hpp"
#include "../vspan.hpp"
#include <vk_mem_alloc.h>
#include <span>

namespace tt::pipeline_box {

/*! Pipeline for rendering simple box shaded quats.
 */
class pipeline_box : public pipeline_vulkan {
public:
    vspan<vertex> vertexBufferData;

    pipeline_box(gui_window const &window);
    ~pipeline_box() {};

    pipeline_box(const pipeline_box &) = delete;
    pipeline_box &operator=(const pipeline_box &) = delete;
    pipeline_box(pipeline_box &&) = delete;
    pipeline_box &operator=(pipeline_box &&) = delete;

    void drawInCommandBuffer(vk::CommandBuffer commandBuffer) override;

protected:
    push_constants pushConstants;

    vk::Buffer vertexBuffer;
    VmaAllocation vertexBufferAllocation;

    std::vector<vk::PipelineShaderStageCreateInfo> createShaderStages() const override;
    std::vector<vk::DescriptorSetLayoutBinding> createDescriptorSetLayoutBindings() const override;
    std::vector<vk::WriteDescriptorSet> createWriteDescriptorSet() const override;
    virtual ssize_t getDescriptorSetVersion() const override;
    std::vector<vk::PushConstantRange> createPushConstantRanges() const override;
    vk::VertexInputBindingDescription createVertexInputBindingDescription() const override;
    std::vector<vk::VertexInputAttributeDescription> createVertexInputAttributeDescriptions() const override;

private:
    void buildvertexBuffers() override;
    void teardownvertexBuffers() override;
};

}
