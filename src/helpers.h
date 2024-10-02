// Ottocento Engine. Architectural BIM Engine.
// Copyright (C) 2024  Lucas M. Faria.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be usefuFl,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
#pragma once
#include <vector>

#include "device.h"
#include "window.h"

//----------------------------------------------------------------------------
/** Majorly extracted from the vulkan-tutorial by Alexander Overvoorde 
 *  and refactored into VkHelper functions format. **/
namespace VkHelpers 
{    
    void generateMipmaps        (VkImage& image, VkFormat imageFormat, int32_t texWidth,  int32_t texHeight, uint32_t mipLevels,
                                 OttDevice& appDevice);
    
    void transitionImageLayout  (VkImage image,      VkFormat format,       VkImageLayout oldLayout,   VkImageLayout newLayout,
                                 uint32_t mipLevels, OttDevice& appDevice);
    
    void createImage            (uint32_t width,  uint32_t height, uint32_t mipLevels,           VkSampleCountFlagBits numSamples,
                                 VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                                 VkImage& image,  VkDeviceMemory& imageMemory,  OttDevice& appDevice);
    
    void create1x1BlankImage    (VkImage& blankImage, uint32_t mipLevels, OttDevice& appDevice,
                                 std::vector<VkImage>& textureImages, VkDeviceMemory& textureImageMemory);
    
    VkShaderModule          createShaderModule      (const std::vector<char>& code, VkDevice device);
    VkSurfaceFormatKHR      chooseSwapSurfaceFormat (const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR        chooseSwapPresentMode   (const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D              chooseSwapExtent        (const VkSurfaceCapabilitiesKHR& capabilities, OttWindow* appwindow);
    
} //namespace VkHelpers


