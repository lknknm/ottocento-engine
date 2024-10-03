#pragma once
#include <volk.h>
#include <string>

// texelImages should represent structs that are 
struct texelImage
{
    uint32_t       mipLevels;
    VkImage        textureImage        = VK_NULL_HANDLE;
    VkDeviceMemory textureImageMemory  = VK_NULL_HANDLE;
    VkImageView    textureImageView    = VK_NULL_HANDLE;
    VkSampler      textureSampler      = VK_NULL_HANDLE;
    
    void createTextureImage(std::string imagePath);
    void createTextureImageView();
    VkImageView createImageView();
    
};