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

#include <algorithm>
#include <array>
#include <cstring>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <unordered_map>
#include <optional>

#include "stb_image.h"

#include "tiny_obj_loader.h"

#include "application.h"
#include "helpers.h"
#include "macros.h"
#include "utils.hxx"

//----------------------------------------------------------------------------
/** Initiates Window and Vulkan related resources to get to the mainLoop.
 *  Cleans resources after the application is closed inside the mainLoop. **/
void OttApplication::run()
{
    initWindow();
    initVulkan();
    mainLoop();
    cleanupVulkanResources();
}

//----------------------------------------------------------------------------
// Main Pipeline functions
//----------------------------------------------------------------------------
    
//----------------------------------------------------------------------------
/** Initiate GLFW window with specific parameters and sets up the window icon.
 *  - Windows-specific: Refresh window to darkmode.
 *  - AppWindow: This will assign the appwindow and  initiate the callbacks to the OttWindow GLFW wrapper.
 *  
 *  This function can potentially be transfered to the window class, with a different class
 *  relationship between members. **/
void OttApplication::initWindow()
{
    viewportCamera->appwindow       = &appwindow;
    viewportCamera->windowHandle    = appwindow.getWindowhandle();
    appwindow.OnFramebufferResized  = [&](const glm::ivec2& size)
    {
        appSwapChain.setFramebufferResized(true);
        appSwapChain.setWidth(size.x);
        appSwapChain.setHeight(size.y);
    };
    appwindow.OnWindowRefreshed = [&]()
    {
        vkDeviceWaitIdle(device);
        //Recreate the swap chain with the new extent
        appSwapChain.refreshSwapChain();
        drawFrame();
    };
    appwindow.OnFileDropped = [&](int count, const char** paths)
    {
        vkDeviceWaitIdle(device);
        cleanupModelObjects();
        
        if (!uniformBuffers.empty())
        {
            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
                vkDestroyBuffer (device, uniformBuffers[i],       nullptr);
                vkFreeMemory    (device, uniformBuffersMemory[i], nullptr);
            }
        }
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
        for (int i = 0; i < count; i++)
        {
            loadModel(paths[i]);
            for (const auto& texPath : sceneMaterials.imageTexture_path)
            {
                createTextureImage(texPath);
                createTextureImageView();
            }
            createVertexBuffer();
            createIndexBuffer();
            createUniformBuffers();
            createDescriptorPool();
            createDescriptorSets();
        }
    };
    appwindow.keyCallback = [&](int key, int scancode, int action, int mods)
    {
        if (action == GLFW_RELEASE)
        {
            std::cout << "Key released: " << key << std::endl;
            /** TODO **/
        }
    };
    #ifdef _WIN32
    appwindow.ThemeRefreshDarkMode(appwindow.getWindowhandle());
    #endif
}
    
//----------------------------------------------------------------------------
/** Initiates and creates Vulkan related resources. **/
void OttApplication::initVulkan()
{        
    createObjectDescriptorSetLayout();
    createGridDescriptorSetLayout();
    createGraphicsPipeline();
    
    mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(1, 1)))) + 1;
    const VkDeviceMemory blankImage = VK_NULL_HANDLE;
    textureImageMemory.push_back(blankImage);
    VkHelpers::create1x1BlankImage(textureImage, mipLevels, appDevice, textureImages, textureImageMemory[0]);
    createTextureImageView();
    createTextureSampler();
    
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
}
    
//----------------------------------------------------------------------------
void OttApplication::mainLoop()
{
    while (!appwindow.windowShouldClose())
    {
        OttWindow::update();
        drawFrame();
    }
    vkDeviceWaitIdle(device);
}

//----------------------------------------------------------------------------
void OttApplication::drawFrame()
{
    if (static_cast<float>(appSwapChain.width()) > 0.0f && static_cast<float>(appSwapChain.height()) > 0.0f)
    {
        const auto startTime = std::chrono::high_resolution_clock::now();
        if (const VkCommandBuffer commandBuffer = ottRenderer.beginFrame())
        {        
            ottRenderer.beginSwapChainRenderPass(commandBuffer);
            const float deltaTime = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - startTime).count() * 0.001f * 0.001f * 0.001f;
            
            drawScene(commandBuffer);
            updateUniformBufferCamera(appSwapChain.getCurrentFrame(), deltaTime, appSwapChain.width(), appSwapChain.height());
            
            ottRenderer.endSwapChainRenderPass(commandBuffer);
            ottRenderer.endFrame();
        }
    }
}

//----------------------------------------------------------------------------
void OttApplication::drawScene(VkCommandBuffer command_buffer)
{
    assert(command_buffer == ottRenderer.getCurrentCommandBuffer() &&
          "Can't begin render pass on command buffer from a different frame");
    
    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                        0, 1, &descriptorSets[ottRenderer.getCurrentFrameIndex()], 0, nullptr);
    
    if (!vertices.empty() && !indices.empty())
    {
        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelines.object);
        const VkBuffer vertexBuffers[] = { vertexBuffer };
        const VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(command_buffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(command_buffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        for (auto& m : models)
        {
            push.offset     = m.offset;
            push.color      = m.pushColorID;
            push.textureID  = m.textureID;
            vkCmdPushConstants(command_buffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantData), &push);
            vkCmdDrawIndexed(command_buffer, m.indexCount, 1, m.startIndex, 0, 0);
        }
    }
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelines.grid);
    vkCmdDraw(command_buffer, 6, 1, 0, 0);
}

//-----------------------------------------------------------------------------
void OttApplication::cleanupTextureObjects()
{
    for (uint32_t i = 0; i < textureImageViews.size() - 1; i++)
    {
        if (textureImageViews[i] != VK_NULL_HANDLE) { vkDestroyImageView (device, textureImageViews[i], nullptr); }
        if (textureImages[i]     != VK_NULL_HANDLE) { vkDestroyImage     (device, textureImages[i],     nullptr); }
    }
    textureImageViews.clear();
    if (textureImageView != VK_NULL_HANDLE) { vkDestroyImageView (device, textureImageView, nullptr); }
    if (textureImage != VK_NULL_HANDLE)     { vkDestroyImage     (device, textureImage,     nullptr); }
    for (uint32_t i = 0; i < textureImageMemory.size(); i++)
    {
        if (textureImageMemory[i] != VK_NULL_HANDLE) { vkFreeMemory(device, textureImageMemory[i], nullptr); }
    }
}

//-----------------------------------------------------------------------------
void OttApplication::cleanupUBO() const
{
    if (!uniformBuffers.empty())
    {
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroyBuffer (device, uniformBuffers[i],       nullptr);
            vkFreeMemory    (device, uniformBuffersMemory[i], nullptr);
        }
    }
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    for (const auto& descSet : descriptorSetLayouts)
    {
        vkDestroyDescriptorSetLayout(device, descSet, nullptr);
    }
}

//-----------------------------------------------------------------------------
void OttApplication::cleanupModelObjects() const
{
    if (indexBuffer != VK_NULL_HANDLE)       { vkDestroyBuffer  (device, indexBuffer,       nullptr); }
    if (indexBufferMemory != VK_NULL_HANDLE) { vkFreeMemory     (device, indexBufferMemory, nullptr); }
    
    if (vertexBuffer != VK_NULL_HANDLE)       { vkDestroyBuffer  (device, vertexBuffer,       nullptr); }
    if (vertexBufferMemory != VK_NULL_HANDLE) { vkFreeMemory     (device, vertexBufferMemory, nullptr); }
}
    
//----------------------------------------------------------------------------
/** Cleanup function to destroy all Vulkan allocated resources **/
void OttApplication::cleanupVulkanResources()
{
    cleanupTextureObjects();
    if (textureSampler != VK_NULL_HANDLE)   { vkDestroySampler   (device, textureSampler,   nullptr); }
    cleanupUBO();
    cleanupModelObjects();

    vkDestroyPipeline(device, graphicsPipelines.object, nullptr);
    vkDestroyPipeline(device, graphicsPipelines.grid, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
}
    
//----------------------------------------------------------------------------
VkImageView OttApplication::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
{
    VkImageViewCreateInfo viewInfo {
        .sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image      = image,
        .viewType   = VK_IMAGE_VIEW_TYPE_2D,
        .format     = format,
        .subresourceRange = {
            .aspectMask = aspectFlags,
            .baseMipLevel = 0,
            .levelCount = mipLevels,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    VkImageView imageView;
    if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
        throw std::runtime_error("failed to create texture image view!");
    return imageView;
}

//----------------------------------------------------------------------------
/** A descriptor set specifies the actual buffer or image resources that will be bound to the descriptors,
 *  just like a framebuffer specifies the actual image views to bind to render pass attachments. **/
void OttApplication::createObjectDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding uboLayoutBinding {
        .binding         = 0,
        .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags      = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
    };
    
    VkDescriptorSetLayoutBinding samplerLayoutBinding {
        .binding            = 1,
        .descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount    = TEXTURE_ARRAY_SIZE * MAX_FRAMES_IN_FLIGHT,
        .stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT,
    };
    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
    std::array<VkDescriptorBindingFlags,  bindings.size()> bindingFlags = {};
    
    bindingFlags[1]  = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
    
    VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo {
        .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
        .bindingCount  = static_cast<uint32_t>(bindings.size()),
        .pBindingFlags = bindingFlags.data()
    };
    
    VkDescriptorSetLayoutCreateInfo objectLayoutInfo {
        .sType          = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext          = &bindingFlagsInfo,
        .bindingCount   = static_cast<uint32_t>(bindings.size()),
        .pBindings      = bindings.data(),
    };
    
    VkDescriptorSetLayout objectDescriptorSetLayout;
    
    if(vkCreateDescriptorSetLayout(device, &objectLayoutInfo, nullptr, &objectDescriptorSetLayout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create descriptor set layout!");
    descriptorSetLayouts.push_back(objectDescriptorSetLayout);
}

//----------------------------------------------------------------------------
/** Separate Grid DescriptorSetLayout to render the grid with a procedural shader. **/
void OttApplication::createGridDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding gridBinding {
        .binding         = 0,
        .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags      = VK_SHADER_STAGE_VERTEX_BIT,
    };
    
    const std::array<VkDescriptorSetLayoutBinding, 1> bindings = {gridBinding};
    
    VkDescriptorSetLayoutCreateInfo gridLayoutInfo {
        .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings    = bindings.data(),
    };

    VkDescriptorSetLayout gridDescriptorSetLayout;
    
    if(vkCreateDescriptorSetLayout(device, &gridLayoutInfo, nullptr, &gridDescriptorSetLayout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create descriptor set layout!");
    descriptorSetLayouts.push_back(gridDescriptorSetLayout);
}
    
//----------------------------------------------------------------------------
/** f = fixed-function stage; p = programmable stage.
 *  Input Assembler (f) > Vertex Shader (p) > Tessellation (p) > Geometry Shader >
 *  Rasterization (f) > Fragment Shader (p) > Colour Blending (f) > Framebuffer **/
void OttApplication::createGraphicsPipeline()
{
    auto vertShaderCode     = Utils::readFile("resource/shaders/vert.spv");
    auto fragShaderCode     = Utils::readFile("resource/shaders/frag.spv");
    auto gridVertShaderCode = Utils::readFile("resource/shaders/gridVert.spv");
    auto gridFragShaderCode = Utils::readFile("resource/shaders/gridFrag.spv");

    VkShaderModule vertShaderModule     = VkHelpers::createShaderModule(vertShaderCode,     device);
    VkShaderModule fragShaderModule     = VkHelpers::createShaderModule(fragShaderCode,     device);
    VkShaderModule gridVertShaderModule = VkHelpers::createShaderModule(gridVertShaderCode, device);
    VkShaderModule gridFragShaderModule = VkHelpers::createShaderModule(gridFragShaderCode, device);

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
        .rasterizationSamples  = appDevice.getMSAASamples(),
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
    
    OttCreatePipelineLayout();
    
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
        .renderPass          = appSwapChain.getRenderPass(),
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
void OttApplication::OttCreatePipelineLayout()
{
    VkPushConstantRange pushConstantRange {
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        .offset     = 0,
        .size       = sizeof(PushConstantData),
    };
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo {
        .sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size()),
        .pSetLayouts    = descriptorSetLayouts.data(),
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &pushConstantRange,
    };
    
    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("failed to create pipeline layout!");
}

//----------------------------------------------------------------------------
void OttApplication::loadModel(std::string modelPath)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;
    std::string baseDir = Utils::GetBaseDir(modelPath) + "\\";
            
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str(), Utils::GetBaseDir(modelPath).c_str()))
    {
        LOG_ERROR(warn + err);
        return;
    }

    LOG(DASHED_SEPARATOR);
    std::cout << "Loading Wavefront " << modelPath << std::endl;
    std::cout << "BaseDir " << Utils::GetBaseDir(modelPath).c_str() << std::endl;

    std::unordered_map<OttModel::Vertex, uint32_t> uniqueVertices{};
    
    materials.push_back(tinyobj::material_t());
    
    for (size_t i = 0; i < materials.size() - 1; i++)
    {
        printf("material[%d].diffuse_texname = %s\n", int(i),
               materials[i].diffuse_texname.c_str());
        sceneMaterials.imageTexture_path.clear();
        sceneMaterials.imageTexture_path.push_back(baseDir + materials[i].diffuse_texname);
    }
    
    OttModel::modelObject model
    {
        .startIndex  = static_cast<uint32_t>(indices.size()),
        .startVertex = static_cast<uint32_t>(vertices.size())
    };
    
    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            OttModel::Vertex vertex{};
            
            vertex.pos =
            {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };
            
            vertex.texCoord =
            {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };
            
            vertex.color = {1.0f, 1.0f, 1.0f};
            
            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }
            indices.push_back(uniqueVertices[vertex]);
        }
    }

    model.indexCount  = static_cast<uint32_t>(indices.size()) - model.startIndex;
    model.pushColorID = {Utils::random_nr(0, 1),  Utils::random_nr(0, 1), Utils::random_nr(0, 1)};
    model.textureID   = static_cast<uint32_t>(textureImages.size());
    models.push_back(model);
    
    LOG(DASHED_SEPARATOR);
    std::cout << "VERTEX COUNT: "       << vertices.size()    << std::endl;
    std::cout << "model.startVertex: "  << model.startVertex  << std::endl;
    std::cout << "model.startIndex: "   << model.startIndex   << std::endl;
    std::cout << "model.indexCount: "   << model.indexCount   << std::endl;
    std::cout << "model.textureID "     << model.textureID    << std::endl;
    LOG(DASHED_SEPARATOR);
}

//----------------------------------------------------------------------------
/** Buffers in Vulkan are regions of memory used for
 *  storing arbitrary data that can be read by the graphics card. **/
void OttApplication::createVertexBuffer()
{
    if (!vertices.empty())
    {
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
    
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        appDevice.createBuffer (bufferSize,
                                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                stagingBuffer, stagingBufferMemory
                                );
        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), (size_t) bufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

        appDevice.createBuffer (bufferSize,
                                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                vertexBuffer, vertexBufferMemory);
        appDevice.debugUtilsObjectNameInfoEXT (VK_OBJECT_TYPE_DEVICE_MEMORY, (uint64_t)vertexBufferMemory, CSTR_RED("application::VkDeviceMemory:vertexBufferMemory"));
        appDevice.debugUtilsObjectNameInfoEXT (VK_OBJECT_TYPE_BUFFER, (uint64_t)vertexBuffer, CSTR_RED("application::VkBuffer:vertexBuffer"));
        appDevice.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }
}

//----------------------------------------------------------------------------
void OttApplication::createIndexBuffer()
{
    if (!vertices.empty())
    {
        VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        appDevice.createBuffer (bufferSize,
                                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                stagingBuffer,
                                stagingBufferMemory);
        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, indices.data(), (size_t) bufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

        appDevice.createBuffer (bufferSize,
                                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                indexBuffer,
                                indexBufferMemory);
        appDevice.debugUtilsObjectNameInfoEXT (VK_OBJECT_TYPE_DEVICE_MEMORY, (uint64_t)indexBufferMemory, "application::VkDeviceMemory:indexBufferMemory");
        appDevice.copyBuffer(stagingBuffer, indexBuffer, bufferSize);
    
        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }
}
    
//----------------------------------------------------------------------------
void OttApplication::createTextureImage(std::string imagePath)
{
    int texWidth, texHeight, texChannels;
    LOG(DASHED_SEPARATOR);
    LOG_INFO("Image path: %s", imagePath.c_str());
    stbi_uc* pixels = stbi_load(imagePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;
    mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    appDevice.createBuffer(imageSize,
                           VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                           stagingBuffer,
                           stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(device, stagingBufferMemory);
    stbi_image_free(pixels);

    VkDeviceMemory textureImageMemoryBuffer;
    textureImageMemory.push_back(textureImageMemoryBuffer);
    VkHelpers::createImage (texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory.back(), appDevice
                            );

    VkHelpers::transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                     mipLevels, appDevice);
    appDevice.copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    //transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps
    
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
    
    VkHelpers::generateMipmaps(textureImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels, appDevice);
    textureImages.push_back(textureImage);
}

//----------------------------------------------------------------------------
/** Images are accessed through image views rather than directly, so we need to craete an
 *  image view for the texture image. **/
void OttApplication::createTextureImageView()
{
    textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
    textureImageViews.push_back(textureImageView);
}

//----------------------------------------------------------------------------
/** The sampler is a distinct object that provides an interface to extract colors from a texture.
 *  This is different from many older APIs, which combined texture images and filtering into a single state. **/
void OttApplication::createTextureSampler()
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    
    VkSamplerCreateInfo samplerInfo {
        .sType        = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter    = VK_FILTER_LINEAR,
        .minFilter    = VK_FILTER_LINEAR,
        .mipmapMode   = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias   = 0.0f,
                        
        .anisotropyEnable        = VK_TRUE,
        .maxAnisotropy           = properties.limits.maxSamplerAnisotropy,

        .compareEnable = VK_FALSE,
        .compareOp     = VK_COMPARE_OP_ALWAYS,
        
        .minLod                  = 0.0f,
        .maxLod                  = static_cast<float>(mipLevels),
        .borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE
    };
    
    if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
        throw std::runtime_error("Failed to create texture sampler!");
}

//----------------------------------------------------------------------------
void OttApplication::createUniformBuffers()
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);
    uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        appDevice.createBuffer(bufferSize,
                               VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                               uniformBuffers[i],
                               uniformBuffersMemory[i]);
        vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
        appDevice.debugUtilsObjectNameInfoEXT(VK_OBJECT_TYPE_DEVICE_MEMORY, (uint64_t) uniformBuffersMemory[i], CSTR_RED(" application::VkDeviceMemory:uniformBuffersMemory %i ", i));
    }
}

//----------------------------------------------------------------------------
void OttApplication::createDescriptorPool()
{
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; 
    poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(TEXTURE_ARRAY_SIZE * MAX_FRAMES_IN_FLIGHT * 2);

    VkDescriptorPoolCreateInfo scenePoolInfo {
        .sType          = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets        = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
        .poolSizeCount  = static_cast<uint32_t>(poolSizes.size()),
        .pPoolSizes     = poolSizes.data(),
    };

    if(vkCreateDescriptorPool(device, &scenePoolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
        throw std::runtime_error("Failed to Create Descriptor Pool!");
}

//----------------------------------------------------------------------------
/** imageInfos array: Local scope only. It will take the created textureImageViews
 + the current sampler and provide them to the descriptor writes. **/
void OttApplication::createDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayouts[0]);
    layouts.push_back(descriptorSetLayouts[1]);
    VkDescriptorSetAllocateInfo allocInfo {
        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool     = descriptorPool,
        .descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
        .pSetLayouts        = layouts.data()
    };

    descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

    const VkResult allocateDescriptorSets = vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data());
    if (allocateDescriptorSets != VK_SUCCESS)
    {
        LOG_ERROR("vkAllocateDescriptorSets returned: %i", static_cast<int>(allocateDescriptorSets));
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);
        
        VkDescriptorImageInfo imageInfos[TEXTURE_ARRAY_SIZE] = {};
        
        for (uint32_t j = 0; j < textureImages.size(); j++)
        {
            imageInfos[j].sampler       = textureSampler;
            imageInfos[j].imageView     = textureImageViews[j];
            imageInfos[j].imageLayout   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }
        
        const std::array descriptorWrites  = {
            VkWriteDescriptorSet {
                .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet          = descriptorSets[i],
                .dstBinding      = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pBufferInfo     = &bufferInfo
                },
                        
            VkWriteDescriptorSet {
                .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet          = descriptorSets[i],
                .dstBinding      = 1,
                .dstArrayElement = 0,
                .descriptorCount = static_cast<uint32_t>(textureImages.size()),
                .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .pImageInfo      = imageInfos
                }
        };
        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        appDevice.debugUtilsObjectNameInfoEXT(VK_OBJECT_TYPE_DESCRIPTOR_SET, (uint64_t) descriptorSets[i], CSTR_RED(" application::descriptorSet[%i] ", i));
    }
}

//----------------------------------------------------------------------------
void OttApplication::updateUniformBufferCamera(uint32_t currentImage, float deltaTime, int width, int height)
{        
    UniformBufferObject ubo {
        .model                 = glm::rotate(glm::mat4(1.0f), glm::radians(270.f), glm::vec3(0.0f, 0.0f, 1.0f)),
        .view                  = viewportCamera->recalculateView(deltaTime),
        .proj                  = viewportCamera->perspectiveProjection(width / (float)height),
        .viewProjectionInverse = viewportCamera->inverseProjection (
                                                viewportCamera->perspectiveProjection(width / (float)height),
                                                viewportCamera->recalculateView(deltaTime)
                                                 ),
        .cameraPos             = viewportCamera->getEyePosition(),
    };
    ubo.proj[1][1] *= -1;
    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}