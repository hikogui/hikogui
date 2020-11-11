// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "pipeline_vulkan.hpp"
#include "../vspan.hpp"
#include <vma/vk_mem_alloc.h>
#include <span>

namespace tt::PipelineToneMapper {

/*! Pipeline for rendering simple flat shaded quats.
 */
class PipelineToneMapper : public pipeline_vulkan {
public:
    PipelineToneMapper(Window_base const &window);
    ~PipelineToneMapper() {};

    PipelineToneMapper(const PipelineToneMapper &) = delete;
    PipelineToneMapper &operator=(const PipelineToneMapper &) = delete;
    PipelineToneMapper(PipelineToneMapper &&) = delete;
    PipelineToneMapper &operator=(PipelineToneMapper &&) = delete;

    void drawInCommandBuffer(vk::CommandBuffer commandBuffer) override;

protected:
    std::vector<vk::PipelineShaderStageCreateInfo> createShaderStages() const override;
    std::vector<vk::DescriptorSetLayoutBinding> createDescriptorSetLayoutBindings() const override;
    std::vector<vk::WriteDescriptorSet> createWriteDescriptorSet() const override;
    ssize_t getDescriptorSetVersion() const override;
    vk::PipelineDepthStencilStateCreateInfo getPipelineDepthStencilStateCreateInfo() const override;

};

}
