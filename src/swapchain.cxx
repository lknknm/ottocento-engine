// Ottocento Engine. Architectural BIM Engine.
// Copyright (C) 2024  Lucas M. Faria.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <algorithm>
#include <array>
#include "swapchain.h"

#include "helpers.h"
#include "macros.h"

//----------------------------------------------------------------------------
/** Default OttSwapChain constructor.
 *  \param device_reference: Pointer to the current device instantiated by the application.
 *  \param window_reference: Pointer to the current window handled by the application.
 *  We need the window's width, height and other utilities to create and recreate our Swapchain
 *  to present the image to the screen. **/
OttSwapChain::OttSwapChain(OttDevice* device_reference, OttWindow* window_reference)
{
    appDeviceRef   = device_reference;
    appWindowRef   = window_reference;
    device = appDeviceRef->getDevice();
    createSwapChain();
    createImageViews();
    createRenderPass();
    createColorResources();
    createDepthResources();
    createFramebuffers();
    createSyncObjects();
}

//----------------------------------------------------------------------------
/** Default OttSwapChain destructor. **/
OttSwapChain::~OttSwapChain()
{
    cleanupSwapChain();
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }
    vkDestroyRenderPass(device, renderPass, nullptr);
}

//----------------------------------------------------------------------------
/** We will query our Swapchain format, present mode and extent to create a new
 * SwapChain.
 * - VK_SHARING_MODE_EXCLUSIVE: An image is owned by one queue family at a time and ownership
 * must be explicitly transferred before using it in another queue family. This option offers the best performance.
 * - VK_SHARING_MODE_CONCURRENT: Images can be used across multiple queue families without explicit ownership transfers.
**/
void OttSwapChain::createSwapChain()
{
    SwapChainSupportDetails swapChainSupport = appDeviceRef->querySwapChainSupport(appDeviceRef->getPhysicalDevice());

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR   presentMode   = chooseSwapPresentMode(swapChainSupport.presentModes, VK_PRESENT_MODE_MAILBOX_KHR);
    VkExtent2D         extent        = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
        imageCount = swapChainSupport.capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR createInfo {
                            .sType              = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                            .surface            = appDeviceRef->getSurface(),
                            .minImageCount      = imageCount,
                            .imageFormat        = surfaceFormat.format,
                            .imageColorSpace    = surfaceFormat.colorSpace,
                            .imageExtent        = extent,
                            .imageArrayLayers   = 1,
                            .imageUsage         = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                            .preTransform       = swapChainSupport.capabilities.currentTransform,
                            .compositeAlpha     = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                            .presentMode        = presentMode,
                            .clipped            = VK_TRUE,
    };
    
    QueueFamilyIndices indices = appDeviceRef->findQueueFamilies(appDeviceRef->getPhysicalDevice());
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices   = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices   = nullptr; // Optional
    }
    
    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
        throw std::runtime_error("failed to create swap chain!");
    LOG_INFO("SwapChain Created");
    
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

//-----------------------------------------------------------------------------
/** Cleanup to SwapChain related objects. **/
void OttSwapChain::cleanupSwapChain() const
{
    vkDestroyImageView (device, depthImageView,   nullptr);
    vkDestroyImage     (device, depthImage,       nullptr);
    vkFreeMemory       (device, depthImageMemory, nullptr);

    vkDestroyImageView (device, colorImageView,   nullptr);
    vkDestroyImage     (device, colorImage,       nullptr);
    vkFreeMemory       (device, colorImageMemory, nullptr);
        
    for (size_t i = 0; i < swapChainFramebuffers.size(); i++) 
        vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
    for (size_t i = 0; i < swapChainImageViews.size(); i++)
        vkDestroyImageView(device, swapChainImageViews[i], nullptr);
    vkDestroySwapchainKHR(device, swapChain, nullptr);
}

//-----------------------------------------------------------------------------
/** Every time we resize or minimize the window, we need to destroy and recreate our
 * swap chain with the new size to present (or not) the rendered image **/
void OttSwapChain::recreateSwapChain()
{
    int width  = appWindowRef->getFrameBufferSize().x;
    int height = appWindowRef->getFrameBufferSize().y;
    while (width == 0 || height == 0)
    {
        width = appWindowRef->getFrameBufferSize().x; height = appWindowRef->getFrameBufferSize().y;
        appWindowRef->waitEvents();
    }
    vkDeviceWaitIdle(device);
        
    cleanupSwapChain();
        
    createSwapChain();
    createImageViews();
    createColorResources();
    createDepthResources();
    createFramebuffers();
}

//----------------------------------------------------------------------------
void OttSwapChain::createImageViews()
{
    swapChainImageViews.resize(swapChainImages.size());
    for (uint32_t i = 0; i < swapChainImages.size(); i++)
        swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

//----------------------------------------------------------------------------
VkImageView OttSwapChain::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) const
{
    VkImageViewCreateInfo viewInfo {
        .sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image      = image,
        .viewType   = VK_IMAGE_VIEW_TYPE_2D,
        .format     = format,
        .subresourceRange = {
            .aspectMask = aspectFlags,
            .baseMipLevel = 0,
            .levelCount = mipLevels,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    VkImageView imageView;
    if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
        throw std::runtime_error("failed to create texture image view!");
    return imageView;
}

//----------------------------------------------------------------------------
/** The renderPass object is a wrapper for the information we need to be handled
 * throughout the rendering process, such as how many color, depth buffers or possibly
 * stencil buffers will be and so on.
 * It is called before the creation of the graphics pipelines.\n\n
 * TODO: This function can possibly be rewritten to avoid the hard coding of the struct fields
 * for a more flexible implementation. **/
void OttSwapChain::createRenderPass()
{
    VkAttachmentDescription colorAttachment {
                            .format         = swapChainImageFormat,
                            .samples        = appDeviceRef->getMSAASamples(),
                            .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
                            .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
                            .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                            .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
                            .finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentDescription depthAttachment {
                            .format         = appDeviceRef->findDepthFormat(),
                            .samples        = appDeviceRef->getMSAASamples(),
                            .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
                            .storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                            .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                            .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
                            .finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentDescription colorAttachmentResolve {
                            .format         = swapChainImageFormat,
                            .samples        = VK_SAMPLE_COUNT_1_BIT,
                            .loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                            .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
                            .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                            .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
                            .finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };
    
    std::array<VkAttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };
    
    VkAttachmentReference colorAttachmentRef {
                          .attachment = 0,
                          .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentReference depthAttachmentRef {
                          .attachment = 1,
                          .layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentReference colorAttachmentResolveRef {
                          .attachment = 2,
                          .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkSubpassDescription subpass {
                        .pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
                        .colorAttachmentCount    = 1,
                        .pColorAttachments       = &colorAttachmentRef,
                        .pResolveAttachments     = &colorAttachmentResolveRef,
                        .pDepthStencilAttachment = &depthAttachmentRef,
    };

    VkSubpassDependency dependency {
                        .srcSubpass    = VK_SUBPASS_EXTERNAL,
                        .dstSubpass    = 0,
                        .srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT  | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                        .dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                        .srcAccessMask = 0,
                        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    };

    VkRenderPassCreateInfo renderPassInfo {
                        .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                        .attachmentCount = static_cast<uint32_t>(attachments.size()),
                        .pAttachments    = attachments.data(),
                        .subpassCount    = 1,
                        .pSubpasses      = &subpass,
                        .dependencyCount = 1,
                        .pDependencies   = &dependency,
    };

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
        throw std::runtime_error("failed to create render pass!");
    LOG_INFO("Render Pass Created");
}

//----------------------------------------------------------------------------
void OttSwapChain::createDepthResources()
{
    const VkFormat depthFormat = appDeviceRef->findDepthFormat();
        
    VkHelpers::createImage(swapChainExtent.width, swapChainExtent.height, 1, appDeviceRef->getMSAASamples(), depthFormat,
                            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory,
                            *appDeviceRef);
    depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
    VkHelpers::transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1,
                                     *appDeviceRef);
    appDeviceRef->debugUtilsObjectNameInfoEXT(VK_OBJECT_TYPE_DEVICE_MEMORY, (uint64_t)depthImageMemory, CSTR_RED("SwapChain::VkDeviceMemory:depthImageMemory"));
}


//----------------------------------------------------------------------------
void OttSwapChain::createColorResources()
{
    const VkFormat colorFormat = swapChainImageFormat;
    VkHelpers::createImage (swapChainExtent.width, swapChainExtent.height,
                            1, appDeviceRef->getMSAASamples(),
                            colorFormat, VK_IMAGE_TILING_OPTIMAL,
                            VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                            colorImage, colorImageMemory, *appDeviceRef
                            );
    appDeviceRef->debugUtilsObjectNameInfoEXT(VK_OBJECT_TYPE_DEVICE_MEMORY, (uint64_t)colorImageMemory,CSTR_RED(" SwapChain::VkDeviceMemory:colorImageMemory "));
    colorImageView = createImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

//----------------------------------------------------------------------------
void OttSwapChain::createFramebuffers()
{
    swapChainFramebuffers.resize(swapChainImageViews.size());
    for (size_t i = 0; i < swapChainImageViews.size(); i++)
    {
        std::array<VkImageView, 3> attachments = {
            colorImageView,
            depthImageView,
            swapChainImageViews[i],
        };

        VkFramebufferCreateInfo framebufferInfo {
                                .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                                .renderPass      = renderPass,
                                .attachmentCount = static_cast<uint32_t>(attachments.size()),
                                .pAttachments    = attachments.data(),
                                .width           = swapChainExtent.width,
                                .height          = swapChainExtent.height,
                                .layers          = 1,
        };

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
            throw std::runtime_error("failed to create framebuffer!");
    }
}

//----------------------------------------------------------------------------
/** Creates the objects needed to synchronize the execution and submission of
 *  commands recorded in the commandBuffer to the Device Queue (VkQueue).
 *  - Semaphores: can be used to control resource access across multiple queues or inside the same queue.
 *  This control happens only on the device, without the influence from the CPU (host).
 *  - Fences: can be used to communicate to the host that execution of
 *  some task on the device has completed, controlling resource access between host and device.
 *  Fences must be reset manually to put them back into the unsignaled state.
 *  The host is responsible for resetting the fence. **/
void OttSwapChain::createSyncObjects()
{
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        
    constexpr VkSemaphoreCreateInfo semaphoreInfo {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
        
    constexpr VkFenceCreateInfo fenceInfo {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };
        
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i])                   != VK_SUCCESS
            )
        {
            throw std::runtime_error("failed to create semaphores!");
        }
        appDeviceRef->debugUtilsObjectNameInfoEXT(VK_OBJECT_TYPE_SEMAPHORE, (uint64_t)imageAvailableSemaphores[i], CSTR_RED("SyncObject::VkSemaphore:imageAvailableSemaphore[%i]", i));
        appDeviceRef->debugUtilsObjectNameInfoEXT(VK_OBJECT_TYPE_SEMAPHORE, (uint64_t)renderFinishedSemaphores[i], CSTR_RED("SyncObject::VkSemaphore:renderFinishedSemaphore[%i]", i));
    }
}

//----------------------------------------------------------------------------
/** Wrapper for the vkAcquireNextImageKHR.
 *  \param image_index: Index of the next available presentable image **/
VkResult OttSwapChain::acquireNextImage(uint32_t& image_index)
{
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    return vkAcquireNextImageKHR (device,
                                  swapChain,
                                  UINT64_MAX,
                                  imageAvailableSemaphores[currentFrame],
                                  VK_NULL_HANDLE,
                                  &image_index);
}

//----------------------------------------------------------------------------
/** Submits the current command buffer for execution to the GPU during the endFrame
 *  call from the renderer class. **/
VkResult OttSwapChain::submitCommandBuffer(const VkCommandBuffer* command_buffers, uint32_t image_index)
{    
    VkSemaphore          waitSemaphores[]   = { imageAvailableSemaphores[currentFrame] };
    VkSemaphore          signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
    VkPipelineStageFlags waitStages[]       = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    
    const VkSubmitInfo submitInfo {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,

        .waitSemaphoreCount = 1,
        .pWaitSemaphores    = waitSemaphores,
        .pWaitDstStageMask  = waitStages,
    
        .commandBufferCount = 1,
        .pCommandBuffers    = command_buffers,

        .signalSemaphoreCount = 1,
        .pSignalSemaphores    = signalSemaphores,
    };
    
    if (vkQueueSubmit(appDeviceRef->getGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) 
        throw std::runtime_error("failed to submit draw command buffer!");
    
    VkSwapchainKHR swapChains[] = { swapChain };
    const VkPresentInfoKHR presentInfo {
        .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores    = signalSemaphores,

        .swapchainCount     = 1,
        .pSwapchains        = swapChains,
        .pImageIndices      = &image_index,
        .pResults           = nullptr, // Optional
    };
    
    const VkResult result = vkQueuePresentKHR(appDeviceRef->getPresentQueue(), &presentInfo);
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    
    return result;
}

//----------------------------------------------------------------------------
// Helper Functions
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
/** Surface Format will be the specification of the window surface colour depth. **/
VkSurfaceFormatKHR OttSwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
            && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return availableFormat;
    }
    return availableFormats[0];
}

//----------------------------------------------------------------------------
/** We can choose our swapchain present mode based on what are the available
 *  present modes on the Physical Device. If the desired one is not found, VK_PRESENT_MODE_FIFO_KHR
 *  will be returned instead.
 *  \param availablePresentModes: The available Present Modes queried on the function querySwapChainSupport,
 *  which returns the struct SwapChainSupportDetails with the "presentModes" field.
 *  \param desiredPresentMode: The VkPresentModeKHR we actually want for our application.
 *  Usually VK_PRESENT_MODE_IMMEDIATE_KHR or VK_PRESENT_MODE_MAILBOX_KHR for our current overall implementation.
 *  **/
VkPresentModeKHR OttSwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, VkPresentModeKHR desiredPresentMode)
{
    for (const auto& availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == desiredPresentMode)
            return availablePresentMode;
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

//----------------------------------------------------------------------------
/** "The swap extent is the resolution of the swap chain images and
 *  it's almost always exactly equal to the resolution of the window that we're drawing to in pixels." **/
VkExtent2D OttSwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)())
    {
        return capabilities.currentExtent;
    }
    else
    {
        const int width  = appWindowRef->getFrameBufferSize().x;
        const int height = appWindowRef->getFrameBufferSize().y;

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width  = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

//----------------------------------------------------------------------------
/** Wrapper to group the vkResetFences and vkResetCommandBuffer functions **/
void OttSwapChain::resetFenceResources(VkCommandBuffer command_buffer)
{
    vkResetFences(device, 1, &inFlightFences[currentFrame]);
    vkResetCommandBuffer(command_buffer, 0); 
}