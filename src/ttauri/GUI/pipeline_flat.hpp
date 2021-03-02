// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "pipeline_vulkan.hpp"
#include "pipeline_flat_push_constants.hpp"
#include "pipeline_flat_vertex.hpp"
#include "../vspan.hpp"
#include <vk_mem_alloc.h>
#include <span>

namespace tt::pipeline_flat {

/*! Pipeline for rendering simple flat shaded quats.
 */
class pipeline_flat : public pipeline_vulkan {
public:
    vspan<vertex> vertexBufferData;

    pipeline_flat(gui_window const &window);
    ~pipeline_flat() {};

    pipeline_flat(const pipeline_flat &) = delete;
    pipeline_flat &operator=(const pipeline_flat &) = delete;
    pipeline_flat(pipeline_flat &&) = delete;
    pipeline_flat &operator=(pipeline_flat &&) = delete;

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
