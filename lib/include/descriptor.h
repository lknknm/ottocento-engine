#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include "device.h"

#include <volk.h>
#include <vector>
#include <glm/glm.hpp>
constexpr int TEXTURE_ARRAY_SIZE = 1000;

/** UBO for the main object pipeline. **/
struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    alignas(16) glm::mat4 viewProjectionInverse;
    alignas(16) glm::vec3 cameraPos;
};


namespace OttDescriptor
{
    void createObjectDescriptorSetLayout    (VkDevice device, std::vector<VkDescriptorSetLayout>& descriptor_set_layouts);
    void createGridDescriptorSetLayout      (VkDevice device, std::vector<VkDescriptorSetLayout>& descriptor_set_layouts);
    void createDescriptorPool               (VkDevice device, VkDescriptorPool& descriptor_pool);
    
    void createDescriptorSets               (const std::vector<VkDescriptorSetLayout>& descriptor_set_layouts,
                                            std::vector<VkDescriptorSet>& descriptor_sets,
                                            const VkDevice device,
                                            const VkDescriptorPool descriptor_pool,
                                            const OttDevice& app_device,
                                            const std::vector<VkBuffer>& uniform_buffers,
                                            const std::vector<VkImage>& texture_images,
                                            const VkSampler texture_sampler,
                                            const std::vector<VkImageView>& texture_image_views
                                            );
}