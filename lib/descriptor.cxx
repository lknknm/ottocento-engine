#include "descriptor.h"

#include <array>
#include <vector>
#include "macros.h"
#include "swapchain.h"


//----------------------------------------------------------------------------
/** Creates a descriptor set for an Object rendering pipeline.
 * - uboLayoutBinding: Binds the uniform buffer to the vertex and fragment shader.
 * - samplerLayoutBinding: Binds the image sampler to the fragment shader. This binding is set as
 * VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT. **/
void OttDescriptor::createObjectDescriptorSetLayout(VkDevice device, std::vector<VkDescriptorSetLayout>& descriptor_set_layouts)
{
    constexpr VkDescriptorSetLayoutBinding uboLayoutBinding {
                                            .binding         = 0,
                                            .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                            .descriptorCount = 1,
                                            .stageFlags      = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
    };
    
    constexpr VkDescriptorSetLayoutBinding samplerLayoutBinding {
                                            .binding            = 1,
                                            .descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            .descriptorCount    = TEXTURE_ARRAY_SIZE * MAX_FRAMES_IN_FLIGHT,
                                            .stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT,
    };
    std::array bindings = { uboLayoutBinding, samplerLayoutBinding };
    std::array<VkDescriptorBindingFlags,  bindings.size()> bindingFlags = {};
    
    bindingFlags[1]  = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
    
    const VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo {
                                            .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
                                            .bindingCount  = static_cast<uint32_t>(bindings.size()),
                                            .pBindingFlags = bindingFlags.data()
    };
    
    const VkDescriptorSetLayoutCreateInfo objectLayoutInfo {
                                            .sType          = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                                            .pNext          = &bindingFlagsInfo,
                                            .bindingCount   = static_cast<uint32_t>(bindings.size()),
                                            .pBindings      = bindings.data(),
    };
    
    VkDescriptorSetLayout objectDescriptorSetLayout;

    const VkResult result = vkCreateDescriptorSetLayout(device, &objectLayoutInfo, nullptr, &objectDescriptorSetLayout);
    if(result != VK_SUCCESS)
    {
        LOG_ERROR("vkCreateDescriptorSetLayout returned: %i", static_cast<int>(result));
        throw std::runtime_error("Failed to create descriptor set layout!");
    }
    descriptor_set_layouts.push_back(objectDescriptorSetLayout);
}

//----------------------------------------------------------------------------
/** Creates a descriptor set for the Grid rendering pipeline.
 *  - gridBinding: We need an Uniform Buffer to pass the camera position to the shader
 *  for the grid falloff. Vertex buffer and Fragment shader are dealt internally. **/
void OttDescriptor::createGridDescriptorSetLayout(VkDevice device, std::vector<VkDescriptorSetLayout>& descriptor_set_layouts)
{
    constexpr VkDescriptorSetLayoutBinding gridBinding {
        .binding         = 0,
        .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags      = VK_SHADER_STAGE_VERTEX_BIT,
    };
    
    const std::array<VkDescriptorSetLayoutBinding, 1> bindings = { gridBinding };
    
    VkDescriptorSetLayoutCreateInfo gridLayoutInfo {
        .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings    = bindings.data(),
    };

    VkDescriptorSetLayout gridDescriptorSetLayout;

    const VkResult result = vkCreateDescriptorSetLayout(device, &gridLayoutInfo, nullptr, &gridDescriptorSetLayout);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("vkCreateDescriptorSetLayout returned: %i", static_cast<int>(result));
        throw std::runtime_error("Failed to create descriptor set layout!");
    }
    descriptor_set_layouts.push_back(gridDescriptorSetLayout);
}

//----------------------------------------------------------------------------
/** \param device: Application side instantiated device.
 *  \param descriptor_pool: Application side instantiated descriptor pool handle. **/
void OttDescriptor::createDescriptorPool(VkDevice device, VkDescriptorPool& descriptor_pool)
{
    std::array poolSizes   = {
        VkDescriptorPoolSize {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
        },
        VkDescriptorPoolSize {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = static_cast<uint32_t>(TEXTURE_ARRAY_SIZE * MAX_FRAMES_IN_FLIGHT * 2),
        }
    };

    const VkDescriptorPoolCreateInfo scenePoolInfo {
        .sType          = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets        = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
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
/** imageInfos array: Local scope only. It will take the created textureImageViews
 + the current sampler and provide them to the descriptor writes. **/
void OttDescriptor::createDescriptorSets(const std::vector<VkDescriptorSetLayout>& descriptor_set_layouts,
                                        std::vector<VkDescriptorSet>& descriptor_sets,
                                        const VkDevice device,
                                        const VkDescriptorPool descriptor_pool,
                                        const OttDevice& app_device,
                                        const std::vector<VkBuffer>& uniform_buffers,
                                        const std::vector<VkImage>& texture_images,
                                        const VkSampler texture_sampler,
                                        const std::vector<VkImageView>& texture_image_views)
{
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptor_set_layouts[0]);
    layouts.push_back(descriptor_set_layouts[1]);
    const VkDescriptorSetAllocateInfo allocInfo {
                                .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                                .descriptorPool     = descriptor_pool,
                                .descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
                                .pSetLayouts        = layouts.data()
    };

    descriptor_sets.resize(MAX_FRAMES_IN_FLIGHT);

    const VkResult result = vkAllocateDescriptorSets(device, &allocInfo, descriptor_sets.data());
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("vkAllocateDescriptorSets returned: %i", static_cast<int>(result));
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkDescriptorBufferInfo bufferInfo {
                                .buffer = uniform_buffers[i],
                                .offset = 0,
                                .range  = sizeof(UniformBufferObject),
        };
        
        VkDescriptorImageInfo imageInfos[TEXTURE_ARRAY_SIZE] = {};
        
        for (uint32_t j = 0; j < texture_images.size(); j++)
        {
            imageInfos[j].sampler       = texture_sampler;
            imageInfos[j].imageView     = texture_image_views[j];
            imageInfos[j].imageLayout   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }
        
        std::array descriptorWrites  = {
            VkWriteDescriptorSet {
                .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet          = descriptor_sets[i],
                .dstBinding      = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pBufferInfo     = &bufferInfo
                },
                        
            VkWriteDescriptorSet {
                .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet          = descriptor_sets[i],
                .dstBinding      = 1,
                .dstArrayElement = 0,
                .descriptorCount = static_cast<uint32_t>(texture_images.size()),
                .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .pImageInfo      = imageInfos
                }
        };
        
        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        app_device.debugUtilsObjectNameInfoEXT(VK_OBJECT_TYPE_DESCRIPTOR_SET, (uint64_t) descriptor_sets[i], CSTR_RED(" application::descriptorSet[%i] ", i));
    }
}