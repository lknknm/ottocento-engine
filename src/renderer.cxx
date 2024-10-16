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

#include <array>
#include "renderer.h"

#include <filesystem>

#include "macros.h"

//----------------------------------------------------------------------------
/** OttRenderer default constructor.
*  \param device_reference: A reference of the current device instantiated by the application.
 * \param swapchain_reference: A reference to the swapchain that we will present the images into.
 * **/
OttRenderer::OttRenderer(OttDevice* device_reference, OttSwapChain* swapchain_reference)
{
    deviceRef = device_reference;
    swapchainRef = swapchain_reference;
    currentFrameIndex = swapchainRef->getCurrentFrame();
    createCommandBuffers();
    LOG_INFO("OttRenderer::OttRenderer Created");
}

//----------------------------------------------------------------------------
OttRenderer::~OttRenderer()
{
    vkFreeCommandBuffers(deviceRef->getDevice(), deviceRef->getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
    commandBuffers.clear();
}

//----------------------------------------------------------------------------
/** Commands in Vulkan do not get called directly. This function allocates the commandBuffers in the device's commandPool. \n
 *  Command buffers will be automatically freed when their command pool is destroyed, so we don't need explicit cleanup. \n
 * The level parameter specifies if the allocated command buffers are primary or secondary command buffers:
 * - VK_COMMAND_BUFFER_LEVEL_PRIMARY: Can be submitted to a queue for execution, but cannot be called from other command buffers.
 * - VK_COMMAND_BUFFER_LEVEL_SECONDARY: Cannot be submitted directly, but can be called from primary command buffers.**/
void OttRenderer::createCommandBuffers()
{
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    
    const VkCommandBufferAllocateInfo allocInfo {
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool        = deviceRef->getCommandPool(),
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<uint32_t>(commandBuffers.size()),
    };
    
    if (vkAllocateCommandBuffers(deviceRef->getDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate command buffers!");
}

//----------------------------------------------------------------------------
/** TODO: pass image_index as argument **/
VkCommandBuffer OttRenderer::beginFrame()
{
    assert(!isFrameStarted && "Can't call beginFrame while already in progress");
    
    if (const VkResult result = swapchainRef->acquireNextImage(currentImageIndex))
    {
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            LOG_ERROR("VK_ERROR_OUT_OF_DATE_KHR");
            swapchainRef->refreshSwapChain();
            return nullptr;
        }
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
            throw std::runtime_error("failed to acquire swap chain image!");
    }
    
    isFrameStarted = true;
    const VkCommandBuffer command_buffer = getCurrentCommandBuffer();
    constexpr VkCommandBufferBeginInfo beginInfo {
                            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    };

    swapchainRef->resetFenceResources(command_buffer);
    if (vkBeginCommandBuffer(command_buffer, &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("failed to begin recording command buffer!");
    return command_buffer;
}

//----------------------------------------------------------------------------
void OttRenderer::beginSwapChainRenderPass(VkCommandBuffer command_buffer)
{
    assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
    assert(
        command_buffer == getCurrentCommandBuffer() &&
        "Can't begin render pass on command buffer from a different frame");
        
    VkRenderPassBeginInfo renderPassInfo {
         .sType        = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
         .renderPass   = swapchainRef->getRenderPass(),
         .framebuffer  = swapchainRef->getFrameBuffer(static_cast<int>(currentImageIndex)),
         .renderArea
         {
             .offset = {0, 0},
             .extent = swapchainRef->getSwapChainExtent(),
           },
     };
    
    // The order of clearValues should be identical to the order of attachments.
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.015f, 0.015f, 0.015f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();
    
    vkCmdBeginRenderPass(command_buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    const VkViewport viewport {
        .x          = 0.0f,
        .y          = 0.0f,
        .width      = static_cast<float>(swapchainRef->width()),
        .height     = static_cast<float>(swapchainRef->height()),
        .minDepth   = 0.0f,
        .maxDepth   = 1.0f,
    };
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    const VkRect2D scissor {
        .offset = {0, 0},
        .extent = swapchainRef->getSwapChainExtent(),
    };
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

}

//----------------------------------------------------------------------------
/** Wrapper for the vkCmdEndRenderPass function with assertion and size rules **/
void OttRenderer::endSwapChainRenderPass(VkCommandBuffer command_buffer) const
{
    assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
    assert(
        command_buffer == getCurrentCommandBuffer() &&
        "Can't end render pass on command buffer from a different frame");
    vkCmdEndRenderPass(command_buffer);
}

//----------------------------------------------------------------------------
void OttRenderer::endFrame()
{
    assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
    const VkCommandBuffer commandBuffer = getCurrentCommandBuffer();
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        throw std::runtime_error("failed to record command buffer!");
    
    if (const VkResult result = swapchainRef->submitCommandBuffer(&commandBuffer, currentImageIndex))
    {
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
        {
            framebufferResized = false;
            swapchainRef->refreshSwapChain();
        }
        else if (result != VK_SUCCESS)
            throw std::runtime_error("failed to present swap chain image!");
    }
    isFrameStarted = false;
    currentFrameIndex = (currentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}


