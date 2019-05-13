// Copyright 2019 Pokitec
// All rights reserved.

#include "Window_vulkan.hpp"
#include "Instance_vulkan.hpp"
#include "Device_vulkan.hpp"
#include "TTauri/logging.hpp"
#include "TTauri/utils.hpp"
#include <boost/numeric/conversion/cast.hpp>
#include <vector>

namespace TTauri::GUI {

using namespace std;

Window_vulkan::Window_vulkan(const std::shared_ptr<Window::Delegate> delegate, const std::string title, vk::SurfaceKHR surface) :
    Window(move(delegate), move(title)),
    intrinsic(move(surface))
{
}

Window_vulkan::~Window_vulkan()
{
    try {
        [[gsl::suppress(f.6)]] {
            get_singleton<Instance_vulkan>()->intrinsic.destroySurfaceKHR(intrinsic);
        }
    } catch (...) {
        abort();
    }
}

void Window_vulkan::initialize()
{
    Window::initialize();
    imagePipeline = TTauri::make_shared<PipelineImage::PipelineImage>(shared_from_this());
}

void Window_vulkan::waitIdle()
{
    lock_dynamic_cast<Device_vulkan>(device)->intrinsic.waitForFences({ renderFinishedFence }, VK_TRUE, std::numeric_limits<uint64_t>::max());
}

void Window_vulkan::render()
{
    //LOG_DEBUG("render '%s' state=%i") % title % static_cast<int>(state.value());

    auto oldState = state.try_transition({
        {State::REQUEST_SET_DEVICE, State::ACCEPTED_SET_DEVICE},
        {State::READY_TO_DRAW, State::WAITING_FOR_VSYNC}
        });

    if (!oldState || oldState.value() == State::REQUEST_SET_DEVICE) {
        return;
    }

    auto vulkanDevice = lock_dynamic_cast<Device_vulkan>(device);

    if (!acquiredImageIndex) {
        // swapchain, fence & imageAvailableSemaphore must be externally synchronized.
        uint32_t imageIndex = 0;
        //LOG_DEBUG("acquireNextImage '%s'") % title;
        auto const result = vulkanDevice->intrinsic.acquireNextImageKHR(swapchain, 0, imageAvailableSemaphore, vk::Fence(), &imageIndex);
        //LOG_DEBUG("/acquireNextImage '%s'") % title;

        switch (result) {
        case vk::Result::eSuccess:
            state.transition({{State::WAITING_FOR_VSYNC, State::READY_TO_DRAW}}); // May not be in WAITING_FOR_SYNC anymore.
            acquiredImageIndex = imageIndex;
            break;

        case vk::Result::eSuboptimalKHR:
            LOG_INFO("acquireNextImageKHR() eSuboptimalKHR");
            state.transition({{State::WAITING_FOR_VSYNC, State::SWAPCHAIN_OUT_OF_DATE}});
            return;

        case vk::Result::eErrorOutOfDateKHR:
            LOG_INFO("acquireNextImageKHR() eErrorOutOfDateKHR");
            state.transition({{State::WAITING_FOR_VSYNC, State::SWAPCHAIN_OUT_OF_DATE}});
            return;

        case vk::Result::eTimeout:
            // Don't render, we didn't receive an image.
            LOG_INFO("acquireNextImageKHR() eTimeout");
            state.transition({{State::WAITING_FOR_VSYNC, State::READY_TO_DRAW}});
            return;

        default: BOOST_THROW_EXCEPTION(Window::SwapChainError());
        }
    }

    if (!state.try_transition({{State::READY_TO_DRAW, State::RENDERING}})) {
        return;
    }

    // Wait until previous rendering has finished, before the next rendering.
    // XXX maybe use one for each swapchain image or go to single command buffer.
    vulkanDevice->intrinsic.waitForFences({ renderFinishedFence }, VK_TRUE, std::numeric_limits<uint64_t>::max());

    // Unsignal the fence so we will not modify/destroy the command buffers during rendering.
    vulkanDevice->intrinsic.resetFences({ renderFinishedFence });

    auto const renderFinishedSemaphore = imagePipeline->render(acquiredImageIndex.value(), imageAvailableSemaphore);

    // Signal the fence when all rendering has finished on the graphics queue.
    // When the fence is signaled we can modify/destroy the command buffers.
    vulkanDevice->graphicsQueue.submit(0, nullptr, renderFinishedFence);

    {
        vector<vk::Semaphore> const renderFinishedSemaphores = { renderFinishedSemaphore };
        vector<vk::SwapchainKHR> const presentSwapchains = { swapchain };
        vector<uint32_t> const presentImageIndices = { acquiredImageIndex.value() };
        BOOST_ASSERT(presentSwapchains.size() == presentImageIndices.size());

        // We will try our best to return the acquired image.
        // A call to presentKHR() always releases the resources of the given image index.
        acquiredImageIndex.reset();

        try {
            auto const result = vulkanDevice->presentQueue.presentKHR({
                boost::numeric_cast<uint32_t>(renderFinishedSemaphores.size()), renderFinishedSemaphores.data(),
                boost::numeric_cast<uint32_t>(presentSwapchains.size()), presentSwapchains.data(), presentImageIndices.data()
                });

            switch (result) {
            case vk::Result::eSuccess:
                state.transition_or_throw({{State::RENDERING, State::READY_TO_DRAW}});
                return;

            case vk::Result::eSuboptimalKHR:
                LOG_INFO("presentKHR() eSuboptimalKHR");
                state.transition_or_throw({{State::RENDERING, State::SWAPCHAIN_OUT_OF_DATE}});
                return;

            default:
                LOG_ERROR("presentKHR() unknown result value");
                state.transition_or_throw({{State::RENDERING, State::READY_TO_DRAW}});
                return;
            }

        } catch (const vk::OutOfDateKHRError &) {
            LOG_INFO("presentKHR() eErrorOutOfDateKHR");
            state.transition_or_throw({{State::RENDERING, State::SWAPCHAIN_OUT_OF_DATE}});
            return;
        }

        // Make sure that resources are released by Vulkan by calling
        vulkanDevice->intrinsic.waitIdle();
    }
}
std::tuple<uint32_t, vk::Extent2D, Window_vulkan::State> Window_vulkan::getImageCountExtentAndState()
{
    vk::SurfaceCapabilitiesKHR surfaceCapabilities;
    try {
        surfaceCapabilities = lock_dynamic_cast<Device_vulkan>(device)->physicalIntrinsic.getSurfaceCapabilitiesKHR(intrinsic);

    } catch (const vk::SurfaceLostKHRError &) {
        return {0, {}, State::SURFACE_LOST};
    }

    auto const currentExtentSet =
        (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) &&
        (surfaceCapabilities.currentExtent.height != std::numeric_limits<uint32_t>::max());

    if (!currentExtentSet) {
        LOG_WARNING("getSurfaceCapabilitiesKHR() does not supply currentExtent");
    }

    uint32_t const imageCount = surfaceCapabilities.maxImageCount ?
        std::clamp(defaultNumberOfSwapchainImages, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount) :
        std::max(defaultNumberOfSwapchainImages, surfaceCapabilities.minImageCount);

    vk::Extent2D const imageExtent = currentExtentSet ?
        surfaceCapabilities.currentExtent :
        (vk::Extent2D{
            std::clamp(
                static_cast<uint32_t>(windowRectangle.extent.width()),
                surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width
            ),
            std::clamp(
                static_cast<uint32_t>(windowRectangle.extent.height()),
                surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height
            )
        });

    auto const minimized = imageExtent.width == 0 || imageExtent.height == 0;

    return { imageCount, imageExtent, minimized ? State::MINIMIZED : State::READY_TO_DRAW};
}

Window::State Window_vulkan::buildForDeviceChange()
{
    auto const swapchainAndState = buildSwapchain();
    swapchain = swapchainAndState.first;
    auto const newState = swapchainAndState.second;

    buildRenderPasses();
    buildFramebuffers();
    buildSemaphores();
    imagePipeline->buildForDeviceChange(firstRenderPass, swapchainCreateInfo.imageExtent, swapchainFramebuffers.size());

    return newState;
}

void Window_vulkan::teardownForDeviceChange()
{
    waitIdle();
    imagePipeline->teardownForDeviceChange();
    teardownSemaphores();
    teardownFramebuffers();
    teardownRenderPasses();
    teardownSwapchain();
}

Window::State Window_vulkan::rebuildForSwapchainChange()
{
    auto const imageState = get<2>(getImageCountExtentAndState());

    if (imageState != State::READY_TO_DRAW) {
        // Early exit when window is minimized.
        return imageState;
    }

    waitIdle();

    imagePipeline->teardownForSwapchainChange();
    teardownFramebuffers();

    auto const swapChainAndState = buildSwapchain(swapchain);
    swapchain = swapChainAndState.first;

    buildFramebuffers();
    imagePipeline->buildForSwapchainChange(firstRenderPass, swapchainCreateInfo.imageExtent, swapchainFramebuffers.size());

    return swapChainAndState.second;
}

std::pair<vk::SwapchainKHR, Window::State> Window_vulkan::buildSwapchain(vk::SwapchainKHR oldSwapchain)
{
    auto vulkanDevice = lock_dynamic_cast<Device_vulkan>(device);

    auto const sharingMode = vulkanDevice->graphicsQueueFamilyIndex == vulkanDevice->presentQueueFamilyIndex ?
        vk::SharingMode::eExclusive :
        vk::SharingMode::eConcurrent;

    vector<uint32_t> const sharingQueueFamilyAllIndices = { vulkanDevice->graphicsQueueFamilyIndex, vulkanDevice->presentQueueFamilyIndex };
    vector<uint32_t> const sharingQueueFamilyNoneIndices = {};

    auto const sharingQueueFamilyIndices = sharingMode == vk::SharingMode::eConcurrent ? sharingQueueFamilyAllIndices : sharingQueueFamilyNoneIndices;

    // Creating swapchain images can fail in different ways, we need to keep retrying.
    vk::SwapchainKHR newSwapchain;
    while (true) {
        auto const [imageCount, imageExtent, imageState] = getImageCountExtentAndState();

        if (imageState != State::READY_TO_DRAW) {
            return { oldSwapchain, imageState };
        }

        swapchainCreateInfo = vk::SwapchainCreateInfoKHR(
            vk::SwapchainCreateFlagsKHR(),
            intrinsic,
            imageCount,
            vulkanDevice->bestSurfaceFormat.format,
            vulkanDevice->bestSurfaceFormat.colorSpace,
            imageExtent,
            1, // imageArrayLayers
            vk::ImageUsageFlagBits::eColorAttachment,
            sharingMode,
            boost::numeric_cast<uint32_t>(sharingQueueFamilyIndices.size()),
            sharingQueueFamilyIndices.data(),
            vk::SurfaceTransformFlagBitsKHR::eIdentity,
            vk::CompositeAlphaFlagBitsKHR::eOpaque,
            vulkanDevice->bestSurfacePresentMode,
            VK_TRUE, // clipped
            oldSwapchain
        );
        
        vk::Result const result = vulkanDevice->intrinsic.createSwapchainKHR(&swapchainCreateInfo, nullptr, &newSwapchain);
        // No matter what, the oldSwapchain has been retired after createSwapchainKHR().
        vulkanDevice->intrinsic.destroy(oldSwapchain);
        oldSwapchain = vk::SwapchainKHR();

        switch (result) {
        case vk::Result::eSuccess:
            break;

        case vk::Result::eErrorSurfaceLostKHR:
            return { {}, State::SURFACE_LOST };

        default:
            LOG_WARNING("Could not create swapchain, retrying.");
            continue;
        }

        auto const [checkImageCount, checkImageExtent, checkImageState] = getImageCountExtentAndState();

        if (checkImageState != State::READY_TO_DRAW) {
            return {newSwapchain, checkImageState};
        }

        if ((imageExtent != checkImageExtent) || (imageCount != checkImageCount)) {
            LOG_WARNING("Surface extent or imageCount has changed while creating swapchain, retrying.");
            // The newSwapchain was created succesfully, it is just of the wrong size so use it as the next oldSwapchain.
            oldSwapchain = newSwapchain;
            continue;
        }

        break;
    }

    view->setRectangle({ 0.0, 0.0 }, { swapchainCreateInfo.imageExtent.width, swapchainCreateInfo.imageExtent.height });

    LOG_INFO("Finished building swap chain");
    LOG_INFO(" - extent=%i x %i") % swapchainCreateInfo.imageExtent.width % swapchainCreateInfo.imageExtent.height;
    LOG_INFO(" - colorSpace=%s, format=%s") % vk::to_string(swapchainCreateInfo.imageColorSpace) % vk::to_string(swapchainCreateInfo.imageFormat);
    LOG_INFO(" - presentMode=%s, imageCount=%i") % vk::to_string(swapchainCreateInfo.presentMode) % swapchainCreateInfo.minImageCount;

    return { newSwapchain, State::READY_TO_DRAW };
}

void Window_vulkan::teardownSwapchain()
{
    lock_dynamic_cast<Device_vulkan>(device)->intrinsic.destroy(swapchain);
}

void Window_vulkan::buildFramebuffers()
{
    auto vulkanDevice = lock_dynamic_cast<Device_vulkan>(device);

    swapchainImages = vulkanDevice->intrinsic.getSwapchainImagesKHR(swapchain);
    for (auto image : swapchainImages) {
        auto imageView = vulkanDevice->intrinsic.createImageView({ vk::ImageViewCreateFlags(),
                                                                   image,
                                                                   vk::ImageViewType::e2D,
                                                                   swapchainCreateInfo.imageFormat,
                                                                   vk::ComponentMapping(),
                                                                   { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 } });

        swapchainImageViews.push_back(imageView);

        std::vector<vk::ImageView> attachments = { imageView };

        auto framebuffer = vulkanDevice->intrinsic.createFramebuffer({
            vk::FramebufferCreateFlags(),
            firstRenderPass,
            boost::numeric_cast<uint32_t>(attachments.size()),
            attachments.data(),
            swapchainCreateInfo.imageExtent.width,
            swapchainCreateInfo.imageExtent.height,
            1 // layers
        });
        swapchainFramebuffers.push_back(framebuffer);
    }

    BOOST_ASSERT(swapchainImageViews.size() == swapchainImages.size());
    BOOST_ASSERT(swapchainFramebuffers.size() == swapchainImages.size());
}

void Window_vulkan::teardownFramebuffers()
{
    auto vulkanDevice = lock_dynamic_cast<Device_vulkan>(device);

    for (auto frameBuffer : swapchainFramebuffers) {
        vulkanDevice->intrinsic.destroy(frameBuffer);
    }
    swapchainFramebuffers.clear();

    for (auto imageView : swapchainImageViews) {
        vulkanDevice->intrinsic.destroy(imageView);
    }
    swapchainImageViews.clear();
}

void Window_vulkan::buildRenderPasses()
{
    std::vector<vk::AttachmentDescription> attachmentDescriptions = { {
        vk::AttachmentDescriptionFlags(),
        swapchainCreateInfo.imageFormat,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare, // stencilLoadOp
        vk::AttachmentStoreOp::eDontCare, // stencilStoreOp
        vk::ImageLayout::eUndefined, // initialLayout
        vk::ImageLayout::ePresentSrcKHR // finalLayout
    } };

    std::vector<vk::AttachmentReference> const inputAttachmentReferences = {};

    std::vector<vk::AttachmentReference> const colorAttachmentReferences = { { 0, vk::ImageLayout::eColorAttachmentOptimal } };

    std::vector<vk::SubpassDescription> const subpassDescriptions = { { vk::SubpassDescriptionFlags(),
                                                                        vk::PipelineBindPoint::eGraphics,
                                                                        boost::numeric_cast<uint32_t>(inputAttachmentReferences.size()),
                                                                        inputAttachmentReferences.data(),
                                                                        boost::numeric_cast<uint32_t>(colorAttachmentReferences.size()),
                                                                        colorAttachmentReferences.data() } };

    std::vector<vk::SubpassDependency> const subpassDependency = { { VK_SUBPASS_EXTERNAL,
                                                                     0,
                                                                     vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                                                     vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                                                     vk::AccessFlags(),
                                                                     vk::AccessFlagBits::eColorAttachmentRead |
                                                                         vk::AccessFlagBits::eColorAttachmentWrite } };

    vk::RenderPassCreateInfo const renderPassCreateInfo = {
        vk::RenderPassCreateFlags(),
        boost::numeric_cast<uint32_t>(attachmentDescriptions.size()),
        attachmentDescriptions.data(),
        boost::numeric_cast<uint32_t>(subpassDescriptions.size()),
        subpassDescriptions.data(),
        boost::numeric_cast<uint32_t>(subpassDependency.size()),
        subpassDependency.data()
    };

    auto vulkanDevice = lock_dynamic_cast<Device_vulkan>(device);

    firstRenderPass = vulkanDevice->intrinsic.createRenderPass(renderPassCreateInfo);
    attachmentDescriptions.at(0).loadOp = vk::AttachmentLoadOp::eClear;
    followUpRenderPass = vulkanDevice->intrinsic.createRenderPass(renderPassCreateInfo);
}

void Window_vulkan::teardownRenderPasses()
{
    auto vulkanDevice = lock_dynamic_cast<Device_vulkan>(device);
    vulkanDevice->intrinsic.destroy(firstRenderPass);
    vulkanDevice->intrinsic.destroy(followUpRenderPass);
}

void Window_vulkan::buildSemaphores()
{
    auto vulkanDevice = lock_dynamic_cast<Device_vulkan>(device);

    imageAvailableSemaphore = vulkanDevice->intrinsic.createSemaphore({}, nullptr);

    // This fence is used to wait for the Window and its Pipelines to be idle.
    // It should therefor be signed at the start so that when no rendering has been
    // done it is still idle.
    renderFinishedFence = vulkanDevice->intrinsic.createFence({ vk::FenceCreateFlagBits::eSignaled }, nullptr);
}

void Window_vulkan::teardownSemaphores()
{
    auto vulkanDevice = lock_dynamic_cast<Device_vulkan>(device);
    vulkanDevice->intrinsic.destroy(imageAvailableSemaphore);
    vulkanDevice->intrinsic.destroy(renderFinishedFence);
}



}
