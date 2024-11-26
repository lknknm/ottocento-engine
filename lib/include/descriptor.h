#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include "device.h"

#include <volk.h>
#include <vector>
#include <glm/glm.hpp>
constexpr int TEXTURE_ARRAY_SIZE = 1024;

/** UBO for the main object pipeline. **/
struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 normalMatrix;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    alignas(16) glm::mat4 viewProjectionInverse;
    alignas(16) glm::vec3 cameraPos;
};

/** Wrapper for helper functions related to Vulkan Descriptors. **/
namespace OttDescriptor
{
    VkDescriptorSetLayout createBindlessDescriptorSetLayout (VkDevice device, OttDevice& app_device);
    void                  createDescriptorPool              (VkDevice device, VkDescriptorPool& descriptor_pool);
    
    VkDescriptorSet createDescriptorSet (
        VkDevice device,
        int count,
        VkDescriptorSetLayout descriptor_set_layout,
        VkDescriptorPool descriptor_pool
    );
    void updateDescriptorSet (
        const VkDevice device,
        const OttDevice& app_device,
        const VkDescriptorSet descriptor_set,
        const VkBuffer& uniform_buffer,
        const std::vector<VkImage>& texture_images,
        const VkSampler texture_sampler,
        const std::vector<VkImageView>& texture_image_views
    );
}