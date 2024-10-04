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

#include "helpers.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <set>
#include <stdexcept>


//----------------------------------------------------------------------------
void VkHelpers::generateMipmaps(VkImage& image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels,
                                OttDevice& appDevice)
{
    // Check if image format supports linear blitting
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(appDevice.getPhysicalDevice(), imageFormat, &formatProperties);

    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
        throw std::runtime_error("texture image format does not support linear blitting!");
    
    VkCommandBuffer commandBuffer = appDevice.beginSingleTimeCommands();
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = image;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;
    
        int32_t mipWidth = texWidth;
        int32_t mipHeight = texHeight;

        for (uint32_t i = 1; i < mipLevels; i++)
        {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier);
            
            VkImageBlit blit{};
            blit.srcOffsets[0] = { 0, 0, 0 };
            blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = 1;
            blit.dstOffsets[0] = { 0, 0, 0 };
            blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
            blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = 1;

            vkCmdBlitImage(commandBuffer,
                            image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                            image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            1, &blit,
                            VK_FILTER_LINEAR);
            
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                                0, nullptr,
                                0, nullptr,
                                1, &barrier);

            if (mipWidth > 1) mipWidth /= 2;
            if (mipHeight > 1) mipHeight /= 2;
        }
        
        barrier.subresourceRange.baseMipLevel = mipLevels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

    appDevice.endSingleTimeCommands(commandBuffer);
}

//----------------------------------------------------------------------------
void VkHelpers::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout,
                                      VkImageLayout newLayout, uint32_t mipLevels, OttDevice& appDevice)
{
    VkCommandBuffer commandBuffer = appDevice.beginSingleTimeCommands();
    VkImageMemoryBarrier barrier {
                         .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                         .oldLayout = oldLayout,
                         .newLayout = newLayout,
                         .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                         .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                         .image = image,
                         .subresourceRange {
                            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                            .baseMipLevel = 0,
                            .levelCount = mipLevels,
                            .baseArrayLayer = 0,
                            .layerCount = 1,
                         }
    };

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;
    
    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (appDevice.hasStencilComponent(format)) {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    } else
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    } 
    else
    {
        throw std::invalid_argument("unsuported layout transition!");
    }
    
    vkCmdPipelineBarrier(commandBuffer,
                        sourceStage, destinationStage,
                        0,
                        0, nullptr,
                        0, nullptr,
                        1, &barrier
                        );
    
    appDevice.endSingleTimeCommands(commandBuffer);
}

//----------------------------------------------------------------------------
void VkHelpers::createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples,
                            VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                            VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory,  OttDevice& appDevice)
{
    VkImageCreateInfo imageInfo {
                        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                        .imageType = VK_IMAGE_TYPE_2D,
                        .format = format,
                        .extent {
                            .width = width,
                            .height = height,
                            .depth = 1,
                        },
                        .mipLevels = mipLevels,
                        .arrayLayers = 1,
                        .samples = numSamples,
                        .tiling = tiling,
                        .usage = usage,
                        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };
    
    if (vkCreateImage(appDevice.getDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS)
        throw std::runtime_error("failed to create image!");

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(appDevice.getDevice(), image, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo {
                        .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                        .allocationSize  = memRequirements.size,
                        .memoryTypeIndex = appDevice.findMemoryType(memRequirements.memoryTypeBits, properties),
    };

    if (vkAllocateMemory(appDevice.getDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate image memory!");
    vkBindImageMemory(appDevice.getDevice(), image, imageMemory, 0);
    appDevice.debugUtilsObjectNameInfoEXT(VK_OBJECT_TYPE_DEVICE_MEMORY, (uint64_t)imageMemory, "application::VkDeviceMemory:textureImageMemory");
}

//----------------------------------------------------------------------------
// Creates a 1x1 blank image to populate the 0 index of the textureImages array.
void VkHelpers::create1x1BlankImage(VkImage& blankImage, uint32_t mipLevels, OttDevice& appDevice,
                                    std::vector<VkImage>& textureImages, VkDeviceMemory& textureImageMemory)
{    
    const VkImageCreateInfo imageInfo {
                .sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                .imageType     = VK_IMAGE_TYPE_2D,
                .format        = VK_FORMAT_R8G8B8A8_SRGB,
                .extent        = { .width = 1, .height = 1, .depth = 1 },
                .mipLevels     = mipLevels,
                .arrayLayers   = 1,
                .samples       = VK_SAMPLE_COUNT_1_BIT,
                .tiling        = VK_IMAGE_TILING_OPTIMAL,
                .usage         = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                .sharingMode   = VK_SHARING_MODE_EXCLUSIVE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    if (vkCreateImage(appDevice.getDevice(), &imageInfo, nullptr, &blankImage) != VK_SUCCESS)
        throw std::runtime_error("failed to create image!");

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(appDevice.getDevice(), blankImage, &memRequirements);
    
    const VkMemoryAllocateInfo allocInfo {
                        .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                        .allocationSize  = memRequirements.size,
                        .memoryTypeIndex = appDevice.findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
    };
    
    if (vkAllocateMemory(appDevice.getDevice(), &allocInfo, nullptr, &textureImageMemory) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate image memory!");
    vkBindImageMemory(appDevice.getDevice(), blankImage, textureImageMemory, 0);
    textureImages.push_back(blankImage);
    appDevice.debugUtilsObjectNameInfoEXT(VK_OBJECT_TYPE_DEVICE_MEMORY, (uint64_t)textureImageMemory, "application::VkDeviceMemory:1x1blankImageMemory");
}

//-----------------------------------------------------------------------------
// Helper function will take a buffer with the bytecode as parameter and create a VkShaderModule from it.
VkShaderModule VkHelpers::createShaderModule(const std::vector<char>& code, VkDevice device)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        throw std::runtime_error("failed to create shader module!");
    else
        std::cout << "Shader Module Created" << std::endl;
    
    return shaderModule;
}

//----------------------------------------------------------------------------
// Surface Format will be the specification of the window surface colour depth.
VkSurfaceFormatKHR VkHelpers::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
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
// "VK_PRESENT_MODE_MAILBOX_KHR is a very nice trade-off if energy usage is not a concern.
// On mobile devices, where energy usage is more important,
// you will probably want to use VK_PRESENT_MODE_FIFO_KHR instead"
VkPresentModeKHR VkHelpers::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            return availablePresentMode;
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

//----------------------------------------------------------------------------
// "The swap extent is the resolution of the swap chain images and
// it's almost always exactly equal to the resolution of the window that we're drawing to in pixels"
VkExtent2D VkHelpers::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, OttWindow* appwindow)
{
    if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)())
    {
        return capabilities.currentExtent;
    }
    else
    {
        const int width  = appwindow->getFrameBufferSize().x;
        const int height = appwindow->getFrameBufferSize().y;

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width  = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}
