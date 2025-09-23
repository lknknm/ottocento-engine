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

#include "descriptor.h"

#include <array>
#include <vector>
#include "macros.h"
#include "swapchain.h"

//----------------------------------------------------------------------------
/** Creates a global, general purpose descriptor set for all rendering pipelines.
 * - uboLayoutBinding: Binds the uniform buffer to the vertex and fragment shader.
 * - samplerLayoutBinding: Binds the image sampler to the fragment shader. This binding is set as
 * VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT. **/
VkDescriptorSetLayout OttDescriptor::createBindlessDescriptorSetLayout(VkDevice device, OttDevice& app_device)
{
    TEXTURE_ARRAY_SIZE = app_device.getMaxDescCount();
    
    constexpr VkDescriptorSetLayoutBinding uboLayoutBinding {
        .binding         = 0,
        .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags      = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
    };
    
    VkDescriptorSetLayoutBinding samplerLayoutBinding {
        .binding            = 1,
        .descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount    = TEXTURE_ARRAY_SIZE,
        .stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT,
    };
    
    std::array bindings = { uboLayoutBinding, samplerLayoutBinding };
    std::array<VkDescriptorBindingFlags, 2> bindingFlags = {
       VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT, // uboLayoutBinding
       VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT // samplerLayoutBinding
    };
    
    const VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo {
        .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
        .bindingCount  = static_cast<uint32_t>(bindings.size()),
        .pBindingFlags = bindingFlags.data()
    };

    const VkDescriptorSetLayoutCreateInfo bindlessLayoutInfo {
        .sType          = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext          = &bindingFlagsInfo,
        .bindingCount   = static_cast<uint32_t>(bindings.size()),
        .pBindings      = bindings.data(),
    };

    VkDescriptorSetLayout bindlessDescriptorSetLayout;
    const VkResult result = vkCreateDescriptorSetLayout(device, &bindlessLayoutInfo, nullptr, &bindlessDescriptorSetLayout);
    if(result != VK_SUCCESS)
    {
        LOG_ERROR("vkCreateDescriptorSetLayout returned: {}", static_cast<int>(result));
        throw std::runtime_error("Failed to create descriptor set layout!");
    }
    app_device.debugUtilsObjectNameInfoEXT(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, (uint64_t) bindlessDescriptorSetLayout, CSTR_CYAN(" OttDescriptor::bindlessDescriptorSetLayout" ));
    return bindlessDescriptorSetLayout;
}

//----------------------------------------------------------------------------
/** \param device: Application side instantiated device.
 *  \param descriptor_pool: Application side instantiated descriptor pool handle. **/
void OttDescriptor::createDescriptorPool(VkDevice device, VkDescriptorPool& descriptor_pool)
{
    std::array poolSizes   = {
        VkDescriptorPoolSize {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
        },
        VkDescriptorPoolSize {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = TEXTURE_ARRAY_SIZE,
        }
    };
    LOG_DEBUG("descriptorCount {}", poolSizes[1].descriptorCount);
    const VkDescriptorPoolCreateInfo scenePoolInfo {
        .sType          = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets        = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * 2),
        .poolSizeCount  = static_cast<uint32_t>(poolSizes.size()),
        .pPoolSizes     = poolSizes.data(),
    };

    const VkResult result = vkCreateDescriptorPool(device, &scenePoolInfo, nullptr, &descriptor_pool);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("vkCreateDescriptorPool returned: %i", static_cast<int>(result));
        throw std::runtime_error("Failed to Create Descriptor Pool!");
    }
}

//----------------------------------------------------------------------------
/** Wrapper function to fill the VkDescriptorSetAllocateInfo struct and allocate one or more descriptor sets.
 * \param device: Application side instantiated device.
 * \param count: How many descriptorSets are to be allocated.
 * \param descriptor_set_layout: A desc. set "blueprint" that will be provided to our allocation info.
 * \param descriptor_pool: Application side instantiated descriptor pool from which the set will be allocated from. **/
VkDescriptorSet OttDescriptor::createDescriptorSet(VkDevice device, int count, VkDescriptorSetLayout descriptor_set_layout, VkDescriptorPool descriptor_pool)
{
    std::vector<VkDescriptorSetLayout> layouts(count, descriptor_set_layout);
    const VkDescriptorSetAllocateInfo allocInfo {
        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool     = descriptor_pool,
        .descriptorSetCount = static_cast<uint32_t>(count),
        .pSetLayouts        = layouts.data()
    };
    
    VkDescriptorSet descriptor_set;
    const VkResult result = vkAllocateDescriptorSets(device, &allocInfo, &descriptor_set);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("vkAllocateDescriptorSets returned: {}", static_cast<int>(result));
        throw std::runtime_error("failed to allocate descriptor sets!");
    }
    LOG_DEBUG("Descriptor allocated");
    LOG_DEBUG("Descriptor allocInfo count: {}", allocInfo.descriptorSetCount);
    return descriptor_set;
}

//----------------------------------------------------------------------------
void OttDescriptor::updateDescriptorSet(const VkDevice device,
                                        const OttDevice& app_device,
                                        const VkDescriptorSet descriptor_set,
                                        const VkBuffer& uniform_buffer,
                                        const std::vector<VkImage>& texture_images,
                                        const VkSampler texture_sampler,
                                        const std::vector<VkImageView>& texture_image_views
                                        )
{
    VkDescriptorBufferInfo bufferInfo {
        .buffer = uniform_buffer,
        .offset = 0,
        .range  = sizeof(UniformBufferObject),
    };
    
    VkDescriptorImageInfo imageInfos[2048] = {};
    
    for (uint32_t j = 0; j < texture_images.size(); j++)
    {
        imageInfos[j].sampler       = texture_sampler;
        imageInfos[j].imageView     = texture_image_views[j];
        imageInfos[j].imageLayout   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
    
    std::array descriptorWrites  = {
        VkWriteDescriptorSet {
            .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet          = descriptor_set,
            .dstBinding      = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pBufferInfo     = &bufferInfo
            },
                    
        VkWriteDescriptorSet {
            .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet          = descriptor_set,
            .dstBinding      = 1,
            .dstArrayElement = 0,
            .descriptorCount = static_cast<uint32_t>(texture_images.size()),
            .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo      = imageInfos
            }
    };
    
    vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    app_device.debugUtilsObjectNameInfoEXT(VK_OBJECT_TYPE_DESCRIPTOR_SET, (uint64_t) descriptor_set, CSTR_RED(" application::descriptorSet "));
}