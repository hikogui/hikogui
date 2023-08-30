// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "pipeline_vulkan.hpp"
#include "pipeline_tone_mapper_push_constants.hpp"
#include "../container/module.hpp"
#include "../macros.hpp"
#include <vma/vk_mem_alloc.h>
#include <span>

namespace hi::inline v1::pipeline_tone_mapper {

/*! Pipeline for rendering simple flat shaded quats.
 */
class pipeline_tone_mapper : public pipeline {
public:
    ~pipeline_tone_mapper() = default;
    pipeline_tone_mapper(const pipeline_tone_mapper &) = delete;
    pipeline_tone_mapper &operator=(const pipeline_tone_mapper &) = delete;
    pipeline_tone_mapper(pipeline_tone_mapper &&) = delete;
    pipeline_tone_mapper &operator=(pipeline_tone_mapper &&) = delete;

    pipeline_tone_mapper(gfx_surface *surface) : pipeline(surface) {}

    void draw_in_command_buffer(vk::CommandBuffer commandBuffer, draw_context const& context) override;

protected:
    push_constants _push_constants;

    [[nodiscard]] std::vector<vk::PipelineShaderStageCreateInfo> createShaderStages() const override;
    [[nodiscard]] std::vector<vk::DescriptorSetLayoutBinding> createDescriptorSetLayoutBindings() const override;
    [[nodiscard]] std::vector<vk::WriteDescriptorSet> createWriteDescriptorSet() const override;
    [[nodiscard]] size_t getDescriptorSetVersion() const override;
    [[nodiscard]] std::vector<vk::PushConstantRange> createPushConstantRanges() const override;
    [[nodiscard]] vk::PipelineDepthStencilStateCreateInfo getPipelineDepthStencilStateCreateInfo() const override;
};

} // namespace hi::inline v1::pipeline_tone_mapper
