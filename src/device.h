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

#include <optional>
#include <vector>
#include <string>

#include "window.h"

//----------------------------------------------------------------------------
// Struct declarations
//----------------------------------------------------------------------------

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class OttDevice
{
//----------------------------------------------------------------------------
public:
//----------------------------------------------------------------------------
    #ifdef NDEBUG
    bool enableValidationLayers = false;
    #else
    bool enableValidationLayers = true;
    #endif

    VkPhysicalDeviceProperties properties;

    OttDevice(OttWindow &window);
    ~OttDevice();

    // // Not copyable or movable
    // OttDevice(const OttDevice &) = delete;
    // OttDevice &operator=(const OttDevice &) = delete;
    // OttDevice(OttDevice &&) = delete;
    // OttDevice &operator=(OttDevice &&) = delete;

    VkInstance            getInstance()       const { return instance; }
    VkCommandPool         getCommandPool()    const { return commandPool; }
    VkDevice              getDevice()         const { return device; }
    VkPhysicalDevice      getPhysicalDevice() const { return physicalDevice; }
    VkSurfaceKHR          getSurface()        const { return surface; }
    VkQueue               getGraphicsQueue()  const { return graphicsQueue; }
    VkQueue               getPresentQueue()   const { return presentQueue; }
    VkSampleCountFlagBits getMSAASamples()    const { return msaaSamples; }
    
    SwapChainSupportDetails  querySwapChainSupport   ();
    QueueFamilyIndices       findQueueFamilies       () const;
    VkFormat                 findSupportedFormat     (const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat                 findDepthFormat         ();
    uint32_t                 findMemoryType          (uint32_t typeFilter, VkMemoryPropertyFlags properties);
    
    VkCommandBuffer beginSingleTimeCommands() const;
    
    void endSingleTimeCommands  (VkCommandBuffer commandBuffer);
    void createBuffer           (VkDeviceSize size, VkMemoryPropertyFlags propertiesFlags, VkBufferUsageFlags usage, VkBuffer& buffer,  VkDeviceMemory &bufferMemory);
        
    void copyBuffer             (VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize size);
    void copyBufferToImage      (VkBuffer& buffer,    VkImage& image,      uint32_t width, uint32_t height);

    void debugUtilsObjectNameInfoEXT (VkObjectType objType, uint64_t objHandle, const char* objName) const;
    bool hasStencilComponent         (VkFormat format);
    
//----------------------------------------------------------------------------
private:
//----------------------------------------------------------------------------
    VkInstance               instance;
    VkSurfaceKHR             surface;
    VkDebugUtilsMessengerEXT debugMessenger;

    VkPhysicalDevice         physicalDevice = VK_NULL_HANDLE;
    VkSampleCountFlagBits    msaaSamples    = VK_SAMPLE_COUNT_1_BIT;
    VkDevice                 device;

    VkQueue                  graphicsQueue;
    VkQueue                  presentQueue;
    VkCommandPool            commandPool;

    std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
    std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME };

    void createInstance();
    void setupDebugMessenger();
    void createWindowSurface(const OttWindow& window);
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createCommandPool(VkCommandPoolCreateFlags flags);

//----------------------------------------------------------------------------
// Device Specific Helper functions ------------------------------------------
    
    std::vector<const char*> getRequiredExtensions   () const;
    VkSampleCountFlagBits    getMaxUsableSampleCount () const;

    int  rateDeviceSuitability();
    bool isDeviceSuitable            (VkPhysicalDevice physicalDevice, std::vector<const char*> deviceExtensions);
    bool checkDeviceExtensionSupport (std::vector<const char*> deviceExtensions);
    bool checkValidationLayerSupport (const std::vector<const char*> &validationLayers);

// End of helper functions --------------------------------------------------
    
}; // class OttDevice