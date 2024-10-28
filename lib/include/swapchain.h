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

#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <vector>
#include "device.h"

const int MAX_FRAMES_IN_FLIGHT = 2;

/** Мид: "The first thing is that your swap chain images and your frames in flight are different things.
 * Let your SwapChain class manage the VkSwapchainKHR and its associated VkImageViews and the "rendering's done"
 * VkSemaphore that goes with each of those images (and maybe their associated VkFrameBuffers,
 * if you're not using dynamic rendering, I guess). Everything else belongs somewhere else.
 * Your swap chain should have a function that takes a semaphore that you want it to signal
 * when it's safe to start writing pixels (which goes with your frames-in-flight data) and it returns a struct that's got the
 * framebuffer/image to render into and the semaphore which you must signal when pixels are all written.
 * You call that at the top of the function that draws a frame. And once rendering commands are all submitted
 * you'll call another function that does the present call.
 * Everything else is just managing the lifetimes of the contained resources." **/

/** The swapchain is an infrastructure that we need to create explicitly in Vulkan.
 *  The OttSwapChain class is a wrapper around the Vulkan SwapChain that will
 *  encapsulate all the swapchain and framebuffer related resources.
 *  
 *  "The swap chain is essentially a queue of images that are waiting to be presented to the screen.
 *  Our application will acquire such an image to draw to it, and then return it to the queue."
 *  **/
class OttSwapChain
{
//----------------------------------------------------------------------------
public:
//----------------------------------------------------------------------------
    
    OttSwapChain(OttDevice* device_reference, OttWindow* window_reference);
    ~OttSwapChain();
    
    [[nodiscard]] VkSwapchainKHR  getSwapChain()              const { return swapChain; }
    [[nodiscard]] VkFramebuffer   getFrameBuffer(int index)   const { return swapChainFramebuffers[index]; }
    [[nodiscard]] uint32_t        getCurrentFrame()           const { return currentFrame; }
    [[nodiscard]] VkRenderPass    getRenderPass()             const { return renderPass; }
    [[nodiscard]] VkImageView     getImageView(int index)     const { return swapChainImageViews[index]; }
    [[nodiscard]] VkFormat        getSwapChainImageFormat()   const { return swapChainImageFormat; }
    [[nodiscard]] VkExtent2D      getSwapChainExtent()        const { return swapChainExtent; }
    [[nodiscard]] size_t          imageCount()                const { return swapChainImages.size(); }

    [[nodiscard]] uint32_t        width()                     const { return swapChainExtent.width; }
    [[nodiscard]] uint32_t        height()                    const { return swapChainExtent.height; }
    [[nodiscard]] VkResult        acquireNextImage(uint32_t& image_index);
    [[nodiscard]] VkResult        submitCommandBuffer(const VkCommandBuffer* buffers, uint32_t image_index);
    
    /** Public wrapper to recreateSwapChain(). **/
    void refreshSwapChain()          { return recreateSwapChain(); }
    void resetFenceResources(VkCommandBuffer command_buffer);
    void setWidth(uint32_t size_x)   { swapChainExtent.width = size_x; }
    void setHeight(uint32_t size_y)  { swapChainExtent.height = size_y; }
    void setFramebufferResized(bool resized) { framebufferResized = resized; }
    bool isFramebufferResized()        const { return framebufferResized; }
    
//----------------------------------------------------------------------------
private:
//----------------------------------------------------------------------------
    
    OttDevice* appDeviceRef;
    OttWindow* appWindowRef;
    VkDevice device;
    
    VkSwapchainKHR           swapChain;
    VkFormat                 swapChainImageFormat;
    VkExtent2D               swapChainExtent;
    std::vector<VkImage>     swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    VkRenderPass             renderPass;

    std::vector<VkFramebuffer>      swapChainFramebuffers;

    VkImage                         colorImage;
    VkDeviceMemory                  colorImageMemory = VK_NULL_HANDLE;
    VkImageView                     colorImageView;
    
    VkImage                         depthImage;
    VkDeviceMemory                  depthImageMemory = VK_NULL_HANDLE;
    VkImageView                     depthImageView;

    std::vector<VkSemaphore>        imageAvailableSemaphores;
    std::vector<VkSemaphore>        renderFinishedSemaphores;
    std::vector<VkFence>            inFlightFences;
    
    uint32_t currentFrame = 0;
    bool   framebufferResized = false;
    
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) const;
    
    void createSwapChain();
    void cleanupSwapChain() const;
    void recreateSwapChain();
    void createImageViews();
    void createRenderPass();
    void createDepthResources();
    void createColorResources();
    void createFramebuffers();
    void createSyncObjects();

//----------------------------------------------------------------------------
// SwapChain Specific Helper functions ---------------------------------------

    VkSurfaceFormatKHR      chooseSwapSurfaceFormat (const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR        chooseSwapPresentMode   (const std::vector<VkPresentModeKHR>& availablePresentModes, VkPresentModeKHR desiredPresentMode);
    VkExtent2D              chooseSwapExtent        (const VkSurfaceCapabilitiesKHR& capabilities);
    
// End of helper functions --------------------------------------------------
    
}; // class OttSwapChain