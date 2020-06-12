// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/Pipeline_vulkan.hpp"
#include "TTauri/GUI/globals.hpp"
#include "TTauri/GUI/GUIDevice_forward.hpp"
#include "TTauri/Foundation/vspan.hpp"
#include <vma/vk_mem_alloc.h>
#include <nonstd/span>

namespace tt::PipelineToneMapper {

/*! Pipeline for rendering simple flat shaded quats.
 */
class PipelineToneMapper : public Pipeline_vulkan {
public:
    PipelineToneMapper(Window const &window);
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
