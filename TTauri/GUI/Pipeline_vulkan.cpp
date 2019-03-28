
#include "Pipeline_vulkan.hpp"

#include "Device_vulkan.hpp"
#include "Window_vulkan.hpp"

#include "TTauri/Logging.hpp"
#include "TTauri/utils.hpp"

#include <boost/assert.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/numeric/conversion/cast.hpp>

namespace TTauri { namespace GUI {

using namespace std;

Pipeline_vulkan::Pipeline_vulkan(const std::shared_ptr<Window> &window) :
    Pipeline(window) {}

Pipeline_vulkan::~Pipeline_vulkan()
{
}

vk::Semaphore Pipeline_vulkan::render(uint32_t imageIndex, vk::Semaphore inputSemaphore)
{
    validateCommandBuffer(imageIndex);

    vector<vk::Semaphore> const waitSemaphores = { inputSemaphore };
    vector<vk::PipelineStageFlags> const waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    BOOST_ASSERT(waitSemaphores.size() == waitStages.size());

    vector<vk::Semaphore> const signalSemaphores = { renderFinishedSemaphores.at(imageIndex) };
    vector<vk::CommandBuffer> const commandBuffersToSubmit = { commandBuffers.at(imageIndex) };

    vector<vk::SubmitInfo> const submitInfo = { {
            boost::numeric_cast<uint32_t>(waitSemaphores.size()), waitSemaphores.data(), waitStages.data(),
            boost::numeric_cast<uint32_t>(commandBuffersToSubmit.size()), commandBuffersToSubmit.data(),
            boost::numeric_cast<uint32_t>(signalSemaphores.size()), signalSemaphores.data()
    } };

    device<Device_vulkan>()->graphicsQueue.submit(submitInfo, vk::Fence());

    return renderFinishedSemaphores.at(imageIndex);
}

void Pipeline_vulkan::buildShaders()
{
    shaderModules = createShaderModules();
    shaderStages = createShaderStages(shaderModules);
}

void Pipeline_vulkan::teardownShaders()
{
    auto vulkanDevice = device<Device_vulkan>();

    for (auto shaderModule : shaderModules) {
        vulkanDevice->intrinsic.destroy(shaderModule);
    }
    shaderModules.clear();
    shaderStages.clear();
}



void Pipeline_vulkan::buildCommandBuffers(size_t nrFrameBuffers)
{
    auto vulkanDevice = device<Device_vulkan>();

    commandBuffers = vulkanDevice->intrinsic.allocateCommandBuffers({
        vulkanDevice->graphicsCommandPool, 
        vk::CommandBufferLevel::ePrimary, 
        boost::numeric_cast<uint32_t>(nrFrameBuffers)
    });

    commandBuffersValid.resize(nrFrameBuffers);
    invalidateCommandBuffers(false);
}

void Pipeline_vulkan::teardownCommandBuffers()
{
    auto vulkanDevice = device<Device_vulkan>();

    vulkanDevice->intrinsic.freeCommandBuffers(vulkanDevice->graphicsCommandPool, commandBuffers);
    commandBuffers.clear();
    commandBuffersValid.clear();
}

void Pipeline_vulkan::buildSemaphores(size_t nrFrameBuffers)
{
    auto vulkanDevice = device<Device_vulkan>();

    auto const semaphoreCreateInfo = vk::SemaphoreCreateInfo();
    renderFinishedSemaphores.resize(nrFrameBuffers);
    for (size_t i = 0; i < nrFrameBuffers; i++) {
        renderFinishedSemaphores.at(i) = vulkanDevice->intrinsic.createSemaphore(semaphoreCreateInfo, nullptr);
    }
}

void Pipeline_vulkan::teardownSemaphores()
{
    auto vulkanDevice = device<Device_vulkan>();

    for (size_t i = 0; i < renderFinishedSemaphores.size(); i++) {
        vulkanDevice->intrinsic.destroy(renderFinishedSemaphores.at(i));
    }
    renderFinishedSemaphores.clear();
}

void Pipeline_vulkan::buildPipeline(vk::RenderPass _renderPass, vk::Extent2D extent)
{
    LOG_INFO("buildPipeline (%i, %i)") % extent.width % extent.height;

    renderPass = _renderPass;

    pipelineLayout = createPipelineLayout();

    vertexInputBindingDescription = createVertexInputBindingDescription();

    vertexInputAttributeDescriptions = createVertexInputAttributeDescriptions();

    pipelineVertexInputStateCreateInfo = createPipelineVertexInputStateCreateInfo(vertexInputBindingDescription, vertexInputAttributeDescriptions);

    pipelineInputAssemblyStateCreateInfo = createPipelineInputAssemblyStateCreateInfo();

    viewports = createViewports(extent);

    scissors = createScissors(extent);

    pipelineViewportStateCreateInfo = createPipelineViewportStateCreateInfo(viewports, scissors);

    pipelineRasterizationStateCreateInfo = createPipelineRasterizationStateCreateInfo();

    pipelineMultisampleStateCreateInfo = createPipelineMultisampleStateCreateInfo();

    pipelineColorBlendAttachmentStates = createPipelineColorBlendAttachmentStates();

    pipelineColorBlendStateCreateInfo = createPipelineColorBlendStateCreateInfo(pipelineColorBlendAttachmentStates);

    graphicsPipelineCreateInfo = {
        vk::PipelineCreateFlags(),
        boost::numeric_cast<uint32_t>(shaderStages.size()),
        shaderStages.data(),
        &pipelineVertexInputStateCreateInfo,
        &pipelineInputAssemblyStateCreateInfo,
        nullptr, // tesselationStateCreateInfo
        &pipelineViewportStateCreateInfo,
        &pipelineRasterizationStateCreateInfo,
        &pipelineMultisampleStateCreateInfo,
        nullptr, // pipelineDepthStencilCrateInfo
        &pipelineColorBlendStateCreateInfo,
        nullptr, // pipelineDynamicsStateCreateInfo
        pipelineLayout,
        renderPass,
        0, // subpass
        vk::Pipeline(), // basePipelineHandle
        -1 // basePipelineIndex
    };

    intrinsic = device<Device_vulkan>()->intrinsic.createGraphicsPipeline(vk::PipelineCache(), graphicsPipelineCreateInfo);
    LOG_INFO("/buildPipeline (%i, %i)") % extent.width % extent.height;

}

void Pipeline_vulkan::teardownPipeline()
{
    auto vulkanDevice = device<Device_vulkan>();
    vulkanDevice->intrinsic.destroy(intrinsic);
    vulkanDevice->intrinsic.destroy(pipelineLayout);
}

void Pipeline_vulkan::buildForDeviceChange(vk::RenderPass renderPass, vk::Extent2D extent, size_t nrFrameBuffers)
{
    buildShaders();
    buildVertexBuffers(nrFrameBuffers);
    buildCommandBuffers(nrFrameBuffers);
    buildSemaphores(nrFrameBuffers);
    buildPipeline(renderPass, extent);
}

void Pipeline_vulkan::teardownForDeviceChange()
{
    invalidateCommandBuffers(true);
    teardownPipeline();
    teardownSemaphores();
    teardownCommandBuffers();
    teardownVertexBuffers();
    teardownShaders();
}

void Pipeline_vulkan::buildForSwapchainChange(vk::RenderPass renderPass, vk::Extent2D extent, size_t nrFrameBuffers)
{
    if (nrFrameBuffers != commandBuffers.size()) {
        teardownSemaphores();
        teardownCommandBuffers();
        teardownVertexBuffers();

        buildVertexBuffers(nrFrameBuffers);
        buildCommandBuffers(nrFrameBuffers);
        buildSemaphores(nrFrameBuffers);
    }
    buildPipeline(renderPass, extent);
}

void Pipeline_vulkan::teardownForSwapchainChange()
{
    invalidateCommandBuffers(true);
    teardownPipeline();
}

void Pipeline_vulkan::invalidateCommandBuffers(bool reset)
{
    for (size_t imageIndex = 0; imageIndex < commandBuffersValid.size(); imageIndex++) {
        commandBuffersValid.at(imageIndex) = false;
        if (reset) {
            auto commandBuffer = commandBuffers.at(imageIndex);
            commandBuffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
        }
    }
}

void Pipeline_vulkan::validateCommandBuffer(uint32_t imageIndex)
{
    if (commandBuffersValid.at(imageIndex)) {
        return;
    }

    LOG_INFO("validateCommandBuffer %i (%i, %i)") % imageIndex % scissors.at(0).extent.width % scissors.at(0).extent.height;

    auto commandBuffer = commandBuffers.at(imageIndex);

    commandBuffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);

    commandBuffer.begin({vk::CommandBufferUsageFlagBits::eSimultaneousUse});

    std::array<float, 4> const blackColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    vector<vk::ClearValue> const clearColors = {{blackColor}};

    auto vulkanWindow = lock_dynamic_cast<Window_vulkan>(window);

    commandBuffer.beginRenderPass({
            renderPass, 
            vulkanWindow->swapchainFramebuffers.at(imageIndex), 
            scissors.at(0), 
            boost::numeric_cast<uint32_t>(clearColors.size()),
            clearColors.data()
        }, vk::SubpassContents::eInline
    );

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, intrinsic);

    drawInCommandBuffer(commandBuffer, imageIndex);

    commandBuffer.endRenderPass();

    commandBuffer.end();

    commandBuffersValid.at(imageIndex) = true;
}

vk::ShaderModule Pipeline_vulkan::loadShader(boost::filesystem::path path) const
{
    LOG_INFO("Loading shader %s") % path.filename().generic_string();

    auto tmp_path = path.generic_string();
    boost::interprocess::file_mapping mapped_file(tmp_path.c_str(), boost::interprocess::read_only);
    auto region = boost::interprocess::mapped_region(mapped_file, boost::interprocess::read_only);

    // Check uint32_t alignment of pointer.
    BOOST_ASSERT((reinterpret_cast<std::uintptr_t>(region.get_address()) & 3) == 0);

    return device<Device_vulkan>()->intrinsic.createShaderModule({vk::ShaderModuleCreateFlags(), region.get_size(), static_cast<uint32_t *>(region.get_address())});
}

vk::PipelineLayout Pipeline_vulkan::createPipelineLayout() const
{
    auto pushConstantRanges = createPushConstantRanges();

    return device<Device_vulkan>()->intrinsic.createPipelineLayout({
        vk::PipelineLayoutCreateFlags(),
        0, nullptr,
        boost::numeric_cast<uint32_t>(pushConstantRanges.size()), pushConstantRanges.data()
    });
}

vk::PipelineVertexInputStateCreateInfo Pipeline_vulkan::createPipelineVertexInputStateCreateInfo(
    const vk::VertexInputBindingDescription &vertexBindingDescriptions,
    const std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions) const
{
    return { vk::PipelineVertexInputStateCreateFlags(),
             1,
             &vertexBindingDescriptions,
             boost::numeric_cast<uint32_t>(vertexAttributeDescriptions.size()),
             vertexAttributeDescriptions.data() };
}

vk::PipelineInputAssemblyStateCreateInfo Pipeline_vulkan::createPipelineInputAssemblyStateCreateInfo() const
{
    return { vk::PipelineInputAssemblyStateCreateFlags(), vk::PrimitiveTopology::eTriangleList, VK_FALSE };
}

std::vector<vk::Viewport> Pipeline_vulkan::createViewports(vk::Extent2D extent) const
{
    return { { 0.0f, 0.0f, boost::numeric_cast<float>(extent.width), boost::numeric_cast<float>(extent.height), 0.0f, 1.0f } };
}

std::vector<vk::Rect2D> Pipeline_vulkan::createScissors(vk::Extent2D extent) const
{
    return { { { 0, 0 }, extent } };
}

vk::PipelineViewportStateCreateInfo
Pipeline_vulkan::createPipelineViewportStateCreateInfo(const std::vector<vk::Viewport> &viewports, std::vector<vk::Rect2D> &scissors) const
{
    return { vk::PipelineViewportStateCreateFlags(),
             boost::numeric_cast<uint32_t>(viewports.size()),
             viewports.data(),
             boost::numeric_cast<uint32_t>(scissors.size()),
             scissors.data() };
}

vk::PipelineRasterizationStateCreateInfo Pipeline_vulkan::createPipelineRasterizationStateCreateInfo() const
{
    return {
        vk::PipelineRasterizationStateCreateFlags(),
        VK_FALSE, // depthClampEnable
        VK_FALSE, // rasterizerDiscardEnable
        vk::PolygonMode::eFill,
        vk::CullModeFlagBits::eBack,
        vk::FrontFace::eClockwise,
        VK_FALSE, // depthBiasEnable
        0.0, // depthBiasConstantFactor
        0.0, // depthBiasClamp
        0.0, // depthBiasSlopeFactor
        1.0 // lineWidth
    };
}

vk::PipelineMultisampleStateCreateInfo Pipeline_vulkan::createPipelineMultisampleStateCreateInfo() const
{
    return {
        vk::PipelineMultisampleStateCreateFlags(),
        vk::SampleCountFlagBits::e1,
        VK_FALSE, // sampleShadingEnable
        1.0f, // minSampleShading
        nullptr, // sampleMask
        VK_FALSE, // alphaToCoverageEnable
        VK_FALSE // alphaToOneEnable
    };
}

std::vector<vk::PipelineColorBlendAttachmentState> Pipeline_vulkan::createPipelineColorBlendAttachmentStates() const
{
    return { { VK_FALSE, // blendEnable
               vk::BlendFactor::eOne, // srcColorBlendFactor
               vk::BlendFactor::eZero, // dstColorBlendFactor
               vk::BlendOp::eAdd, // colorBlendOp
               vk::BlendFactor::eOne, // srcAlphaBlendFactor
               vk::BlendFactor::eZero, // dstAlphaBlendFactor
               vk::BlendOp::eAdd, // aphaBlendOp
               vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA } };
}

vk::PipelineColorBlendStateCreateInfo
Pipeline_vulkan::createPipelineColorBlendStateCreateInfo(const std::vector<vk::PipelineColorBlendAttachmentState> &attachements) const
{
    return { vk::PipelineColorBlendStateCreateFlags(),
             VK_FALSE, // logicOpenable
             vk::LogicOp::eCopy,
             boost::numeric_cast<uint32_t>(attachements.size()),
             attachements.data() };
}



}}