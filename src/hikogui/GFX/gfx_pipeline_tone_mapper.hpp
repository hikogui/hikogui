// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "gfx_pipeline_vulkan.hpp"
#include "gfx_pipeline_tone_mapper_push_constants.hpp"
#include "../container/module.hpp"
#include "../macros.hpp"
#include <vma/vk_mem_alloc.h>
#include <span>

namespace hi::inline v1::gfx_pipeline_tone_mapper {

/*! Pipeline for rendering simple flat shaded quats.
 */
class gfx_pipeline_tone_mapper : public gfx_pipeline {
public:
    ~gfx_pipeline_tone_mapper() = default;
    gfx_pipeline_tone_mapper(const gfx_pipeline_tone_mapper&) = delete;
    gfx_pipeline_tone_mapper& operator=(const gfx_pipeline_tone_mapper&) = delete;
    gfx_pipeline_tone_mapper(gfx_pipeline_tone_mapper&&) = delete;
    gfx_pipeline_tone_mapper& operator=(gfx_pipeline_tone_mapper&&) = delete;

    gfx_pipeline_tone_mapper(gfx_surface *surface) : gfx_pipeline(surface) {}

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

} // namespace hi::inline v1::gfx_pipeline_tone_mapper
