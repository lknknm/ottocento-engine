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

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include "macros.h"
#include "model.h"
#include "pipeline.h"

#include "utils.hxx"

OttPipeline::OttPipeline(OttDevice* device_reference, OttSwapChain* swapchain_reference)
{
    pDevice         = device_reference;
    pSwapchain      = swapchain_reference;
    device = pDevice->getDevice();
}

OttPipeline::~OttPipeline()
{
    vkDestroyPipeline       (device, graphicsPipelines.object, nullptr);
    vkDestroyPipeline       (device, graphicsPipelines.grid, nullptr);
    vkDestroyPipeline       (device, graphicsPipelines.wireframe, nullptr);
    vkDestroyPipelineLayout (device, pipelineLayout, nullptr);
    LOG_DEBUG("OttPipeline object destroyed");
}

//----------------------------------------------------------------------------
/** f = fixed-function stage; p = programmable stage.
 *  Input Assembler (f) > Vertex Shader (p) > Tessellation (p) > Geometry Shader >
 *  Rasterization (f) > Fragment Shader (p) > Color Blending (f) > Framebuffer. \n
 *  - Bindings: spacing between data and whether the data is per-vertex or per-instance
 *  - Attribute descriptions: type of the attributes passed to the vertex shader, which binding to land which offset. **/
void OttPipeline::createGraphicsPipeline (
    std::string vertex_shader_path, std::string fragment_shader_path,
    VkPipeline&  pipeline, VkPipelineVertexInputStateCreateInfo vertex_input_info,
    VkPolygonMode polygon_mode
    )
{
    auto vertexShaderCode   = Utils::readFile(vertex_shader_path);
    auto fragShaderCode     = Utils::readFile(fragment_shader_path);

    VkShaderModule vertShaderModule    = createShaderModule(vertexShaderCode);
    VkShaderModule fragShaderModule    = createShaderModule(fragShaderCode);
    std::array     shaderStages        = {
    VkPipelineShaderStageCreateInfo { initShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, vertShaderModule) },
    VkPipelineShaderStageCreateInfo { initShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, fragShaderModule) }};
    
    VkPipelineInputAssemblyStateCreateInfo inputAssembly        = initInputAssembly();
    VkPipelineViewportStateCreateInfo      viewportState        = initViewportState(1, 1);
    VkPipelineRasterizationStateCreateInfo rasterState          = initRasterizer(polygon_mode, 1.0f);
    VkPipelineMultisampleStateCreateInfo   multisampling        = initMultisamplingState(pDevice->getMSAASamples());
    VkPipelineDepthStencilStateCreateInfo  depthStencil         = initDepthStencilInfo();
    VkPipelineColorBlendAttachmentState    colorBlendAttachment = initColorBlendAttachment();
    VkPipelineColorBlendStateCreateInfo    colorBlending        = initColorBlendCreateInfo(&colorBlendAttachment);
    VkPipelineDynamicStateCreateInfo       dynamicState         = initDynamicState();
        
    // Populate the Graphics Pipeline Info struct.
    // First referencing the array of VkPipelineShaderStageCreateInfo structs.
    VkGraphicsPipelineCreateInfo pipelineInfo {
        .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount          = 2,
        .pStages             = shaderStages.data(),
        .pVertexInputState   = &vertex_input_info,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState      = &viewportState,
        .pRasterizationState = &rasterState,
        .pMultisampleState   = &multisampling,
        .pDepthStencilState  = &depthStencil,
        .pColorBlendState    = &colorBlending,
        .pDynamicState       = &dynamicState,
        .layout              = pipelineLayout,
        .renderPass          = pSwapchain->getRenderPass(),
        .subpass             = 0,
        .basePipelineHandle  = VK_NULL_HANDLE,
    };

    VkResult result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo,nullptr, &pipeline);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("vkCreateGraphicsPipelines returned: %i", static_cast<int>(result));
        throw std::runtime_error("Failed to create graphics pipeline.");
    }
    LOG_INFO("Pipeline Created");
    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

//----------------------------------------------------------------------------
/** Wrapper to create a pipeline layout and pass it to our pipeline creation stage
 * \param push_stage_flags: Specifies for which shader stage the push constants should be passed to.  **/
void OttPipeline::createPipelineLayout(VkShaderStageFlags push_stage_flags, std::vector<VkDescriptorSetLayout>& descriptor_set_layouts)
{
    VkPushConstantRange pushConstantRange {
        .stageFlags = push_stage_flags,
        .offset     = 0,
        .size       = sizeof(PushConstantData),
    };
    
    const VkPipelineLayoutCreateInfo pipelineLayoutInfo {
        .sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size()),
        .pSetLayouts    = descriptor_set_layouts.data(),
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &pushConstantRange,
    };

    const VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("vkCreatePipelineLayout returned: %i", static_cast<int>(result));
        throw std::runtime_error("Failed to create pipeline layout");
    }
    pDevice->debugUtilsObjectNameInfoEXT(VK_OBJECT_TYPE_PIPELINE_LAYOUT, (uint64_t)pipelineLayout, CSTR_RED("OttPipeline::VkPipelineLayout:pipelineLayout"));
    LOG_INFO("OttPipeline::pipelineLayout created.");
}

//-----------------------------------------------------------------------------
/** Helper function will take a buffer with the bytecode as parameter and create a VkShaderModule from it. **/
VkShaderModule OttPipeline::createShaderModule(const std::vector<char>& code)
{
    const VkShaderModuleCreateInfo createInfo {
                            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                            .codeSize = code.size(),
                            .pCode = reinterpret_cast<const uint32_t*>(code.data()),
    };

    VkShaderModule shaderModule;
    const VkResult result = vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("vkCreateShaderModule returned: %i", static_cast<int>(result));
        throw std::runtime_error("failed to create shader module!");
    }
    LOG_INFO("Shader Module Created");
    return shaderModule;
}

//----------------------------------------------------------------------------
// Struct initialization Helper functions ------------------------------------
//----------------------------------------------------------------------------
/** Helper function to initialize a VkPipelineShaderStageCreateInfo struct. **/
constexpr VkPipelineShaderStageCreateInfo OttPipeline::initShaderStageCreateInfo(VkShaderStageFlagBits stage, VkShaderModule module)
{
    return VkPipelineShaderStageCreateInfo  {
        .sType   = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage   = stage,
        .module  = module,
        .pName   = "main",
    };
}

//-----------------------------------------------------------------------------
/** Helper function to initialize a VkPipelineVertexInputStateCreateInfo struct. **/
VkPipelineVertexInputStateCreateInfo OttPipeline::initVertexInputInfo   (uint32_t vertex_binding_desc_count,
                                                                        VkVertexInputBindingDescription* binding_description,
                                                                        uint32_t att_desc_count,
                                                                        VkVertexInputAttributeDescription* vertex_att_desc)
{
    return VkPipelineVertexInputStateCreateInfo {
        .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount   = vertex_binding_desc_count,
        .pVertexBindingDescriptions      = binding_description,
        .vertexAttributeDescriptionCount = att_desc_count,
        .pVertexAttributeDescriptions    = vertex_att_desc
    };
}


//-----------------------------------------------------------------------------
/** Helper function to initialize a VkPipelineInputAssemblyStateCreateInfo struct. **/
constexpr VkPipelineInputAssemblyStateCreateInfo OttPipeline::initInputAssembly()
{
    return VkPipelineInputAssemblyStateCreateInfo {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };
}

//-----------------------------------------------------------------------------
/** Helper function to initialize a VkPipelineViewportStateCreateInfo struct. **/
constexpr VkPipelineViewportStateCreateInfo OttPipeline::initViewportState(uint32_t viewport_count,uint32_t scissor_count)
{
    return VkPipelineViewportStateCreateInfo {
        .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = viewport_count,
        .scissorCount  = scissor_count,
    };
}

//-----------------------------------------------------------------------------
/** Helper function to initialize a VkPipelineRasterizationStateCreateInfo struct. **/
constexpr VkPipelineRasterizationStateCreateInfo OttPipeline::initRasterizer(VkPolygonMode polygon_mode, float line_width)
{
    return VkPipelineRasterizationStateCreateInfo {
        .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable        = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode             = polygon_mode,
        .cullMode                = VK_CULL_MODE_NONE,
        .frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable         = VK_FALSE,
        .lineWidth               = line_width,
    };
}

//-----------------------------------------------------------------------------
/** Helper function to initialize a VkPipelineMultisampleStateCreateInfo struct. **/
constexpr VkPipelineMultisampleStateCreateInfo OttPipeline::initMultisamplingState(VkSampleCountFlagBits rasterization_samples)
{
    return VkPipelineMultisampleStateCreateInfo {
        .sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples  = rasterization_samples,
        .sampleShadingEnable   = VK_TRUE,
        .minSampleShading      = 0.2f, 
        .pSampleMask           = nullptr, 
        .alphaToCoverageEnable = VK_FALSE, 
        .alphaToOneEnable      = VK_FALSE, 
    };
}

//-----------------------------------------------------------------------------
/** Helper function to initialize a VkPipelineDepthStencilStateCreateInfo struct. **/
constexpr VkPipelineDepthStencilStateCreateInfo OttPipeline::initDepthStencilInfo()
{
    return VkPipelineDepthStencilStateCreateInfo  {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable        = VK_TRUE,
        .depthWriteEnable       = VK_TRUE,
        .depthCompareOp         = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable  = VK_FALSE,
        .minDepthBounds = 0.0f,
        .maxDepthBounds = 1.0f,
    };
}

//-----------------------------------------------------------------------------
/** Helper function to initialize a VkPipelineColorBlendAttachmentState struct. **/
constexpr VkPipelineColorBlendAttachmentState OttPipeline::initColorBlendAttachment()
{
    return VkPipelineColorBlendAttachmentState {
        .blendEnable            = VK_TRUE,
        .srcColorBlendFactor    = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor    = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp           = VK_BLEND_OP_ADD, 
        .srcAlphaBlendFactor    = VK_BLEND_FACTOR_ONE, 
        .dstAlphaBlendFactor    = VK_BLEND_FACTOR_ZERO, 
        .alphaBlendOp           = VK_BLEND_OP_ADD, 
        .colorWriteMask         = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };
}

//-----------------------------------------------------------------------------
/** Helper function to initialize a VkPipelineColorBlendStateCreateInfo struct. **/
constexpr VkPipelineColorBlendStateCreateInfo OttPipeline::initColorBlendCreateInfo(VkPipelineColorBlendAttachmentState* color_blend_attachment)
{
    return VkPipelineColorBlendStateCreateInfo {
        .sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable     = VK_FALSE,
        .logicOp           = VK_LOGIC_OP_COPY, 
        .attachmentCount   = 1,
        .pAttachments      = color_blend_attachment,
        .blendConstants = {
            float { 0.0f },
            float { 0.0f },
            float { 0.0f },
            float { 0.0f }
        }
    };
}

//-----------------------------------------------------------------------------
/** Helper function to initialize a VkPipelineDynamicStateCreateInfo struct. **/
constexpr VkPipelineDynamicStateCreateInfo OttPipeline::initDynamicState()
{
    return VkPipelineDynamicStateCreateInfo {
        .sType              = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount  = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates     = dynamicStates.data(),
    };
}

// End of helper functions --------------------------------------------------