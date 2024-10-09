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

#include <memory>
#include <string>
#include <vector>

#include "device.h"
#include "volk.h"

const int MAX_FRAMES_IN_FLIGHT = 2;

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

    VkFramebuffer   getFrameBuffer(int index)   const { return swapChainFramebuffers[index]; }
    VkRenderPass    getRenderPass()             const { return renderPass; }
    VkImageView     getImageView(int index)     const { return swapChainImageViews[index]; }
    size_t          imageCount()                const { return swapChainImages.size(); }
    VkFormat        getSwapChainImageFormat()   const { return swapChainImageFormat; }
    VkExtent2D      getSwapChainExtent()        const { return swapChainExtent; }
    uint32_t        width()                     const { return swapChainExtent.width; }
    uint32_t        height()                    const { return swapChainExtent.height; }
    
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
    VkPresentModeKHR        chooseSwapPresentMode   (const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D              chooseSwapExtent        (const VkSurfaceCapabilitiesKHR& capabilities);
    
// End of helper functions --------------------------------------------------
    
}; // class OttSwapChain