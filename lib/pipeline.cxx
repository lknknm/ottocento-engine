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
    vkDestroyPipelineLayout (device, pipelineLayout, nullptr);
    LOG_DEBUG("OttPipeline object destroyed");
}

//----------------------------------------------------------------------------
/** f = fixed-function stage; p = programmable stage.
 *  Input Assembler (f) > Vertex Shader (p) > Tessellation (p) > Geometry Shader >
 *  Rasterization (f) > Fragment Shader (p) > Colour Blending (f) > Framebuffer **/
void OttPipeline::createGraphicsPipeline(std::vector<VkDescriptorSetLayout>& descriptor_set_layouts)
{
    auto vertShaderCode     = Utils::readFile("resource/shaders/object_vertex.spv");
    auto fragShaderCode     = Utils::readFile("resource/shaders/object_fragment.spv");
    auto gridVertShaderCode = Utils::readFile("resource/shaders/grid_vertex.spv");
    auto gridFragShaderCode = Utils::readFile("resource/shaders/grid_fragment.spv");

    VkShaderModule vertShaderModule     = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule     = createShaderModule(fragShaderCode);
    VkShaderModule gridVertShaderModule = createShaderModule(gridVertShaderCode);
    VkShaderModule gridFragShaderModule = createShaderModule(gridFragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo {
        .sType   = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage   = VK_SHADER_STAGE_VERTEX_BIT,
        .module  = vertShaderModule,
        .pName   = "main",
    };

    VkPipelineShaderStageCreateInfo fragShaderStageInfo {
        .sType   = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage   = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module  = fragShaderModule,
        .pName   = "main",
    };
    
    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{};
    shaderStages[0] = vertShaderStageInfo;
    shaderStages[1] = fragShaderStageInfo;

    // Bindings: spacing between data and whether the data is per-vertex or per-instance.
    // Attribute descriptions: type of the attributes passed to the vertex shader, which binding to load
    // and which offset.
    auto bindingDescription                         = OttModel::Vertex::getBindingDescription();
    auto attributeDescriptions                      = OttModel::Vertex::getAttributeDescriptions();
    VkPipelineVertexInputStateCreateInfo vertexInputInfo {
        .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount   = 1,
        .pVertexBindingDescriptions      = &bindingDescription,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
        .pVertexAttributeDescriptions    = attributeDescriptions.data(),
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssembly {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    VkPipelineViewportStateCreateInfo viewportState {
        .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount  = 1,
    };

    VkPipelineRasterizationStateCreateInfo rasterizer {
        .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable        = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode             = VK_POLYGON_MODE_FILL,
        .cullMode                = VK_CULL_MODE_NONE,
        .frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable         = VK_FALSE,
        .lineWidth               = 1.0f, // if using any other mode than fill.
    };

    VkPipelineMultisampleStateCreateInfo multisampling {
        .sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples  = pDevice->getMSAASamples(),
        .sampleShadingEnable   = VK_TRUE,
        .minSampleShading      = 0.2f, 
        .pSampleMask           = nullptr, 
        .alphaToCoverageEnable = VK_FALSE, 
        .alphaToOneEnable      = VK_FALSE, 
    };

    VkPipelineDepthStencilStateCreateInfo depthStencil {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable        = VK_TRUE,
        .depthWriteEnable       = VK_TRUE,
        .depthCompareOp         = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable  = VK_FALSE,
        .minDepthBounds = 0.0f,
        .maxDepthBounds = 1.0f,
    };
    
    VkPipelineColorBlendAttachmentState colorBlendAttachment {
        .blendEnable            = VK_TRUE,
        .srcColorBlendFactor    = VK_BLEND_FACTOR_ONE, // Optional
        .dstColorBlendFactor    = VK_BLEND_FACTOR_ZERO, // Optional
        .colorBlendOp           = VK_BLEND_OP_ADD, // Optional
        .srcAlphaBlendFactor    = VK_BLEND_FACTOR_ONE, // Optional
        .dstAlphaBlendFactor    = VK_BLEND_FACTOR_ZERO, // Optional
        .alphaBlendOp           = VK_BLEND_OP_ADD, // Optional
        .colorWriteMask         = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };

    VkPipelineColorBlendStateCreateInfo colorBlending {
        .sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable     = VK_FALSE,
        .logicOp           = VK_LOGIC_OP_COPY, // Optional
        .attachmentCount   = 1,
        .pAttachments      = &colorBlendAttachment,
    };
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };

    VkPipelineDynamicStateCreateInfo dynamicState {
        .sType              = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount  = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates     = dynamicStates.data(),
    };
    
    createPipelineLayout(descriptor_set_layouts);
    
    // Populate the Graphics Pipeline Info struct.
    // First referencing the array of VkPipelineShaderStageCreateInfo structs.
    VkGraphicsPipelineCreateInfo pipelineInfo {
        .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount          = 2,
        .pStages             = shaderStages.data(),
        .pVertexInputState   = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState      = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState   = &multisampling,
        .pDepthStencilState  = &depthStencil,
        .pColorBlendState    = &colorBlending,
        .pDynamicState       = &dynamicState,
        .layout              = pipelineLayout,
        .renderPass          = pSwapchain->getRenderPass(),
        .subpass             = 0,
        .basePipelineHandle  = VK_NULL_HANDLE,
    };
    
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE,
                                1,
                                &pipelineInfo,
                                nullptr,
                                &graphicsPipelines.object) != VK_SUCCESS)
                                    throw std::runtime_error("failed to create graphics pipeline!");
    LOG_INFO("Object Pipeline Created");

    //----------------------------------------------------------------------------
    // Grid Pipeline Creation ----------------------------------------------------
    
    VkPipelineShaderStageCreateInfo gridVertShaderStageInfo {
        .sType   = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage   = VK_SHADER_STAGE_VERTEX_BIT,
        .module  = gridVertShaderModule,
        .pName   = "main",
    };
    
    VkPipelineShaderStageCreateInfo gridFragShaderStageInfo{};
    gridFragShaderStageInfo.sType   = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    gridFragShaderStageInfo.stage   = VK_SHADER_STAGE_FRAGMENT_BIT;
    gridFragShaderStageInfo.module  = gridFragShaderModule;
    gridFragShaderStageInfo.pName   = "main";

    shaderStages[0] = gridVertShaderStageInfo;
    shaderStages[1] = gridFragShaderStageInfo;

    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    pipelineInfo.pStages = shaderStages.data();

    vertexInputInfo.vertexBindingDescriptionCount   = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions      = VK_NULL_HANDLE;
    vertexInputInfo.pVertexAttributeDescriptions    = VK_NULL_HANDLE;
    
    if (vkCreateGraphicsPipelines  (device, VK_NULL_HANDLE,
                                   1,
                                   &pipelineInfo,
                                   nullptr,
                                   &graphicsPipelines.grid) != VK_SUCCESS )
    {
        throw std::runtime_error("failed to create graphics pipeline!");
    }
    LOG_INFO("Grid Pipeline Created");
    
    /** Shader modules are just a thin wrapper around the shader
     *  bytecode that we've previously loaded from a file and the functions defined in it.
     *  That means that we're allowed to destroy the shader modules again
     *  as soon as pipeline creation is finished **/
    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
    vkDestroyShaderModule(device, gridVertShaderModule, nullptr);
    vkDestroyShaderModule(device, gridFragShaderModule, nullptr);
}

//----------------------------------------------------------------------------
void OttPipeline::createPipelineLayout(std::vector<VkDescriptorSetLayout>& descriptor_set_layouts)
{
    VkPushConstantRange pushConstantRange {
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
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
        throw std::runtime_error("failed to create pipeline layout!");
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

