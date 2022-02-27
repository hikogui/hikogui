// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "pipeline_vulkan.hpp"
#include "pipeline_tone_mapper_push_constants.hpp"
#include "../vspan.hpp"
#include <vk_mem_alloc.h>
#include <span>

namespace tt::inline v1::pipeline_tone_mapper {

/*! Pipeline for rendering simple flat shaded quats.
 */
class pipeline_tone_mapper : public pipeline_vulkan {
public:
    pipeline_tone_mapper(gfx_surface const &surface);
    ~pipeline_tone_mapper(){};

    pipeline_tone_mapper(const pipeline_tone_mapper &) = delete;
    pipeline_tone_mapper &operator=(const pipeline_tone_mapper &) = delete;
    pipeline_tone_mapper(pipeline_tone_mapper &&) = delete;
    pipeline_tone_mapper &operator=(pipeline_tone_mapper &&) = delete;

    void drawInCommandBuffer(vk::CommandBuffer commandBuffer, draw_context const &context) override;

protected:
    push_constants _push_constants;

    std::vector<vk::PipelineShaderStageCreateInfo> createShaderStages() const override;
    std::vector<vk::DescriptorSetLayoutBinding> createDescriptorSetLayoutBindings() const override;
    std::vector<vk::WriteDescriptorSet> createWriteDescriptorSet() const override;
    ssize_t getDescriptorSetVersion() const override;
    std::vector<vk::PushConstantRange> createPushConstantRanges() const override;
    vk::PipelineDepthStencilStateCreateInfo getPipelineDepthStencilStateCreateInfo() const override;
};

} // namespace tt::inline v1::pipeline_tone_mapper
