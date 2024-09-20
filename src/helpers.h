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

//----------------------------------------------------------------------------
// Majorly extracted from the vulkan-tutorial by Alexander Overvoorde 
// and refactored into VkHelper functions format.
namespace VkHelpers 
{
    void endSingleTimeCommands  (VkCommandBuffer commandBuffer, VkQueue graphicsQueue, VkCommandPool commandPool, VkDevice device);
    
    void generateMipmaps        (VkImage image, VkFormat imageFormat, int32_t texWidth,  int32_t texHeight, uint32_t mipLevels,
                                 VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, VkCommandPool commandPool, VkDevice device);
    
    void createBuffer           (VkDeviceSize size, VkBufferUsageFlags usage,     VkMemoryPropertyFlags properties,
                                 VkBuffer &buffer,  VkDeviceMemory &bufferMemory, VkDevice device,
                                 VkPhysicalDevice physicalDevice);
    
    void copyBuffer             (VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size,               VkQueue graphicsQueue, VkCommandPool commandPool, VkDevice device);
    void copyBufferToImage      (VkBuffer buffer,    VkImage image,      uint32_t width, uint32_t height, VkQueue graphicsQueue, VkCommandPool commandPool, VkDevice device);
    
    void transitionImageLayout  (VkImage image,      VkFormat format,       VkImageLayout oldLayout,   VkImageLayout newLayout,
                                 uint32_t mipLevels, VkQueue graphicsQueue, VkCommandPool commandPool, VkDevice device);
    
    void createImage            (uint32_t width,  uint32_t height, uint32_t mipLevels,           VkSampleCountFlagBits numSamples,
                                 VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                                 VkImage& image,  VkDeviceMemory& imageMemory,  VkDevice device, VkPhysicalDevice physicalDevice);
    
    void create1x1BlankImage    (VkImage& blankImage, uint32_t mipLevels, VkDevice device, VkPhysicalDevice physicalDevice,
                                 std::vector<VkImage>& textureImages, VkDeviceMemory textureImageMemory);
    
    void     debugUtilsObjectNameInfoEXT   (VkDevice device, VkObjectType objType, uint64_t objHandle, const char* objName);
    void     DestroyDebugUtilsMessengerEXT (VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
    VkResult CreateDebugUtilsMessengerEXT  (VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    
    VkCommandBuffer         beginSingleTimeCommands (VkCommandPool    commandPool, VkDevice device);
    VkSampleCountFlagBits   getMaxUsableSampleCount (VkPhysicalDevice physicalDevice);
    QueueFamilyIndices      findQueueFamilies       (VkPhysicalDevice device, VkSurfaceKHR surface);
    VkFormat                findSupportedFormat     (const std::vector<VkFormat>& candidates, VkImageTiling tiling,
                                                     VkFormatFeatureFlags features,           VkPhysicalDevice physicalDevice);
    
    VkFormat                findDepthFormat         (VkPhysicalDevice physicalDevice);
    uint32_t                findMemoryType          (uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDevice);
    
    VkShaderModule          createShaderModule      (const std::vector<char>& code, VkDevice device);
    VkSurfaceFormatKHR      chooseSwapSurfaceFormat (const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR        chooseSwapPresentMode   (const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D              chooseSwapExtent        (const VkSurfaceCapabilitiesKHR& capabilities, OttWindow* appwindow);
    SwapChainSupportDetails querySwapChainSupport   (VkPhysicalDevice device, VkSurfaceKHR surface);
    
    int  rateDeviceSuitability       (VkPhysicalDevice device);
    bool hasStencilComponent         (VkFormat format);
    bool isDeviceSuitable            (VkPhysicalDevice device, VkSurfaceKHR surface, std::vector<const char*> deviceExtensions);
    bool checkDeviceExtensionSupport (VkPhysicalDevice device, std::vector<const char*> deviceExtensions);
    bool checkValidationLayerSupport (const std::vector<const char*> &validationLayers);
    
    VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                 VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                 const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                 void* pUserData);
    
    std::vector<const char*> getRequiredExtensions  (bool enableValidationLayers);
    std::vector<char>        readFile               (const std::string& filename);
} //namespace VkHelpers


