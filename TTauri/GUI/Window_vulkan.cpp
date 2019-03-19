
#include "Window_vulkan.hpp"

#include "Device_vulkan.hpp"

#include "TTauri/Logging.hpp"
#include "TTauri/utils.hpp"

#include <boost/numeric/conversion/cast.hpp>

#include <vector>

namespace TTauri {
namespace GUI {

using namespace std;

Window_vulkan::Window_vulkan(std::shared_ptr<Window::Delegate> delegate, const std::string &title, vk::SurfaceKHR surface) :
    Window(delegate, title),
    intrinsic(surface)
{
}

void Window_vulkan::waitIdle()
{
    lock_dynamic_cast<Device_vulkan>(device)->intrinsic.waitForFences(1, &renderFinishedFence, VK_TRUE, std::numeric_limits<uint64_t>::max());
}

std::pair<uint32_t, vk::Extent2D> Window_vulkan::getImageCountAndImageExtent()
{
    auto const surfaceCapabilities = lock_dynamic_cast<Device_vulkan>(device)->physicalIntrinsic.getSurfaceCapabilitiesKHR(intrinsic);

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
            std::clamp(windowRectangle.extent.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width),
            std::clamp(windowRectangle.extent.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height) });

    return { imageCount, imageExtent };
}

bool Window_vulkan::isOnScreen()
{
    auto const imageCountAndImageExtent = getImageCountAndImageExtent();

    return imageCountAndImageExtent.second.width > 0 && imageCountAndImageExtent.second.height > 0;
}

void Window_vulkan::buildForDeviceChange()
{
    std::scoped_lock lock(mutex);

    if (state == State::LINKED_TO_DEVICE) {
        auto const swapchainAndOnScreen = buildSwapchain();
        swapchain = swapchainAndOnScreen.first;
        auto const onScreen = swapchainAndOnScreen.second;

        buildRenderPasses();
        buildFramebuffers();
        buildSemaphores();
        backingPipeline->buildForDeviceChange(firstRenderPass, swapchainCreateInfo.imageExtent, swapchainFramebuffers.size());

        state = onScreen ? State::READY_TO_DRAW : State::MINIMIZED;

    } else {
        BOOST_THROW_EXCEPTION(Window::StateError());
    }
}

void Window_vulkan::teardownForDeviceChange()
{
    std::scoped_lock lock(mutex);

    if (state == State::READY_TO_DRAW || state == State::SWAPCHAIN_OUT_OF_DATE || state == State::MINIMIZED) {
        waitIdle();
        backingPipeline->teardownForDeviceChange();
        teardownSemaphores();
        teardownFramebuffers();
        teardownRenderPasses();
        teardownSwapchain();

        state = State::LINKED_TO_DEVICE;
    } else {
        BOOST_THROW_EXCEPTION(Window::StateError());
    }
}

bool Window_vulkan::rebuildForSwapchainChange()
{
    if (!isOnScreen()) {
        // Early exit when window is minimized.
        return false;
    }

    waitIdle();

    backingPipeline->teardownForSwapchainChange();
    teardownFramebuffers();

    auto const swapChainAndOnScreen = buildSwapchain(swapchain);
    swapchain = swapChainAndOnScreen.first;
    auto const onScreen = swapChainAndOnScreen.second;

    buildFramebuffers();
    backingPipeline->buildForSwapchainChange(firstRenderPass, swapchainCreateInfo.imageExtent, swapchainFramebuffers.size());

    return onScreen;
}

std::pair<vk::SwapchainKHR, bool> Window_vulkan::buildSwapchain(vk::SwapchainKHR oldSwapchain)
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
        auto const imageCountAndImageExtent = getImageCountAndImageExtent();
        auto const imageCount = imageCountAndImageExtent.first;
        auto const imageExtent = imageCountAndImageExtent.second;

        if (imageExtent.width == 0 || imageExtent.height == 0) {
            return { oldSwapchain, false };
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
            oldSwapchain);
        
        vk::Result const result = vulkanDevice->intrinsic.createSwapchainKHR(&swapchainCreateInfo, nullptr, &newSwapchain);
        // No matter what, the oldSwapchain has been retired after createSwapchainKHR().
        vulkanDevice->intrinsic.destroy(oldSwapchain);
        oldSwapchain = vk::SwapchainKHR();

        if (result != vk::Result::eSuccess) {
            LOG_WARNING("Could not create swapchain, retrying.");
            continue;
        }

        auto const checkImageCountAndImageExtent = getImageCountAndImageExtent();
        auto const checkImageExtent = checkImageCountAndImageExtent.second;
        if (imageExtent != checkImageExtent) {
            LOG_WARNING("Surface extent changed while creating swapchain, retrying.");
            // The newSwapchain was created succesfully, it is just of the wrong size so use it as the next oldSwapchain.
            oldSwapchain = newSwapchain;
            continue;
        }

        break;
    }

    view->setRectangle({ 0.0, 0.0, 0.0 }, { swapchainCreateInfo.imageExtent.width, swapchainCreateInfo.imageExtent.height, 0.0 });

    LOG_INFO("Building swap chain");
    LOG_INFO(" - extent=%i x %i") % swapchainCreateInfo.imageExtent.width % swapchainCreateInfo.imageExtent.height;
    LOG_INFO(" - colorSpace=%s, format=%s") % vk::to_string(swapchainCreateInfo.imageColorSpace) % vk::to_string(swapchainCreateInfo.imageFormat);
    LOG_INFO(" - presentMode=%s, imageCount=%i") % vk::to_string(swapchainCreateInfo.presentMode) % swapchainCreateInfo.minImageCount;

    return { newSwapchain, true };
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

bool Window_vulkan::render(bool blockOnVSync)
{
    uint64_t const timeout = blockOnVSync ? std::numeric_limits<uint64_t>::max() : 0;
    auto vulkanDevice = lock_dynamic_cast<Device_vulkan>(device);

    uint32_t imageIndex = 0;
    {
        auto const result = vulkanDevice->intrinsic.acquireNextImageKHR(swapchain, timeout, imageAvailableSemaphore, vk::Fence(), &imageIndex);
        switch (result) {
        case vk::Result::eSuccess: break;
        case vk::Result::eSuboptimalKHR: LOG_INFO("acquireNextImageKHR() eSuboptimalKHR"); return false;
        case vk::Result::eErrorOutOfDateKHR: LOG_INFO("acquireNextImageKHR() eErrorOutOfDateKHR"); return false;
        case vk::Result::eTimeout:
            // Don't render, we didn't receive an image.
            return true;
        default: BOOST_THROW_EXCEPTION(Window::SwapChainError());
        }
    }

    // Make a fence that should be signaled when all drawing is finished.
    vulkanDevice->intrinsic.waitIdle();
    // device->intrinsic.waitForFences(1, &renderFinishedFence, VK_TRUE, std::numeric_limits<uint64_t>::max());

    vulkanDevice->intrinsic.resetFences({ renderFinishedFence });
    vulkanDevice->graphicsQueue.submit(0, nullptr, renderFinishedFence);

    {
        vector<vk::Semaphore> const renderFinishedSemaphores = { backingPipeline->render(imageIndex, imageAvailableSemaphore) };
        vector<vk::SwapchainKHR> const presentSwapchains = { swapchain };
        vector<uint32_t> const presentImageIndices = { imageIndex };
        BOOST_ASSERT(presentSwapchains.size() == presentImageIndices.size());

        try {
            auto const result = vulkanDevice->presentQueue.presentKHR({
                boost::numeric_cast<uint32_t>(renderFinishedSemaphores.size()), renderFinishedSemaphores.data(),
                boost::numeric_cast<uint32_t>(presentSwapchains.size()), presentSwapchains.data(), presentImageIndices.data()
            });

            switch (result) {
            case vk::Result::eSuccess:
                return true;
            case vk::Result::eSuboptimalKHR:
                LOG_INFO("presentKHR() eSuboptimalKHR");
                return false;
            default:
                LOG_ERROR("presentKHR() unknown result value");
                return false;
            }
            
        } catch (const vk::OutOfDateKHRError &e) {
            LOG_INFO("presentKHR() eErrorOutOfDateKHR");
            return false;
        }
    }
}

}}