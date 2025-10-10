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
#include <stdexcept>
#include <unordered_map>

#include "stb_image.h"
#include "tiny_obj_loader.h"

#include "fmtfs.hxx"
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
            createIndexBuffer(indices, indexBuffer, indexBufferMemory);
            createIndexBuffer(edges, edgesBuffer, edgesBufferMemory);
            edgesBufferAddressInfo.buffer = edgesBuffer;
            createUniformBuffers();
            
            OttDescriptor::createDescriptorPool(device, descriptorPool);
            bindlessDescriptorSet = OttDescriptor::createDescriptorSet(device, 1, bindlessDescSetLayout, descriptorPool);
            OttDescriptor::updateDescriptorSet (
                device, appDevice,
                bindlessDescriptorSet,
                uniformBuffers[0],
                textureImages,
                textureSampler,
                textureImageViews
             );
        }
    };
    appwindow.interactorKeyCallback = [&](int key, int scancode, int action, int mods)
    {
        vkDeviceWaitIdle(device);
        if (action == GLFW_PRESS)
        {
            switch (key)
            {
            case GLFW_KEY_1:
                appPipeline.setDisplayMode(OttPipeline::DISPLAY_MODE_WIREFRAME);
                break;
            case GLFW_KEY_2:
                appPipeline.setDisplayMode(OttPipeline::DISPLAY_MODE_SOLID);
                break;
            case GLFW_KEY_3:
                appPipeline.setDisplayMode(OttPipeline::DISPLAY_MODE_TEXTURE);
                break;
            }
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
    // Pipeline Initilization.    
        auto bindingDescription     = OttModel::Vertex::getBindingDescription();
        auto attributeDescriptions  = OttModel::Vertex::getAttributeDescriptions();
        VkPipelineVertexInputStateCreateInfo modelVertexInputInfo = appPipeline.initVertexInputInfo(1, &bindingDescription, static_cast<uint32_t>(attributeDescriptions.size()), attributeDescriptions.data());
        VkPipelineVertexInputStateCreateInfo gridVertexInputInfo  = appPipeline.initVertexInputInfo(0, VK_NULL_HANDLE, 0, VK_NULL_HANDLE);
        
        appPipeline.createPipelineLayout    (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, &bindlessDescSetLayout);
        appPipeline.createGraphicsPipeline  ("build/shaders/object.vert.spv", "build/shaders/solid_shading.frag.spv",
                                            appPipeline.graphicsPipelines.solid, modelVertexInputInfo, VK_POLYGON_MODE_FILL,
                                            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
                                            );
        appPipeline.createGraphicsPipeline  ("build/shaders/object.vert.spv", "build/shaders/texture.frag.spv",
                                            appPipeline.graphicsPipelines.texture, modelVertexInputInfo, VK_POLYGON_MODE_FILL,
                                            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
                                            );
        appPipeline.createGraphicsPipeline  ("build/shaders/object.vert.spv", "build/shaders/wireframe.frag.spv",
                                            appPipeline.graphicsPipelines.wireframe, modelVertexInputInfo, VK_POLYGON_MODE_LINE,
                                            VK_PRIMITIVE_TOPOLOGY_LINE_LIST
                                            );
        appPipeline.createGraphicsPipeline  ("build/shaders/grid.vert.spv", "build/shaders/grid.frag.spv",
                                            appPipeline.graphicsPipelines.grid, gridVertexInputInfo, VK_POLYGON_MODE_FILL,
                                            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
                                            );
    // endof Pipeline initialization.

    // Textures initilization.
        VkHelpers::create1x1BlankImage(textureImage, mipLevels, appDevice, textureImages, textureImageMemory[0]);
        createTextureImageView();
    
        createTextureImage("resource/matcap/clay_brown.png");
        createTextureImageView();
    
        createTextureImage("resource/matcap/ceramic_lightbulb.png");
        createTextureImageView();
    
        createTextureSampler();
        createUniformBuffers();
    // endof Textures initilization.

    // Descriptor Initilization.
        OttDescriptor::createDescriptorPool(device, descriptorPool);
        bindlessDescriptorSet = OttDescriptor::createDescriptorSet(device, 1, bindlessDescSetLayout, descriptorPool);
        OttDescriptor::updateDescriptorSet (
            device, appDevice,
            bindlessDescriptorSet,
            uniformBuffers[0],
            textureImages,
            textureSampler,
            textureImageViews
        );
    // endof Descriptor Initilization.
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

    vkCmdBindDescriptorSets (
        command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS, appPipeline.getPipelineLayout(),
        0, 1, &bindlessDescriptorSet, 0, nullptr
    );
    
    if (!vertices.empty() && !indices.empty())
    {
        const VkBuffer vertexBuffers[] = { vertexBuffer };
        const VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(command_buffer, 0, 1, vertexBuffers, offsets);
        
        /** TODO: Eliminate switch case with an unordered_map **/
        switch (appPipeline.getDisplayMode())
        {
            case OttPipeline::DISPLAY_MODE_WIREFRAME:
                vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, appPipeline.graphicsPipelines.wireframe);
                vkCmdBindIndexBuffer(command_buffer, edgesBuffer, 0, VK_INDEX_TYPE_UINT32);
                break;
            case OttPipeline::DISPLAY_MODE_SOLID:
                vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, appPipeline.graphicsPipelines.solid);
                vkCmdBindIndexBuffer(command_buffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
                break;
            case OttPipeline::DISPLAY_MODE_TEXTURE:
                vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, appPipeline.graphicsPipelines.texture);
                vkCmdBindIndexBuffer(command_buffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
                break;
        }

        /** TODO: general cleanup for draft shading **/
        for (auto& m : models)
        {
            push.offset     = m.offset;
            push.color      = m.pushColorID;
            push.textureID  = m.textureID;
            vkCmdPushConstants(command_buffer, appPipeline.getPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantData), &push);
            vkCmdDrawIndexed(command_buffer, m.edgeCount, 1, m.startEdge, 0, 0);
        }
        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, appPipeline.graphicsPipelines.texture);
        vkCmdBindIndexBuffer(command_buffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        for (auto& m : models)
        {
            push.offset     = m.offset;
            push.color      = m.pushColorID;
            push.textureID  = m.textureID;
            vkCmdPushConstants(command_buffer, appPipeline.getPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantData), &push);
            vkCmdDrawIndexed(command_buffer, m.indexCount, 1, m.startIndex, 0, 0);
        }

    }
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, appPipeline.graphicsPipelines.grid);
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
    vkDestroyDescriptorSetLayout(device, bindlessDescSetLayout, nullptr);
}

//-----------------------------------------------------------------------------
void OttApplication::cleanupModelObjects() const
{
    if (edgesBuffer != VK_NULL_HANDLE)       { vkDestroyBuffer  (device, edgesBuffer,       nullptr); }
    if (edgesBufferMemory != VK_NULL_HANDLE) { vkFreeMemory     (device, edgesBufferMemory, nullptr); }
    
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
void OttApplication::loadModel(std::filesystem::path const& modelPath)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;
    auto baseDir = modelPath.parent_path();
            
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.string().c_str(), baseDir.string().c_str()))
    {
		LOG_ERROR("{}, {}", warn, err);
		return;
    }

    LOG(DASHED_SEPARATOR);
    fmt::print("Loading Wavefront {}\n", modelPath);
    fmt::print("BaseDir {}\n", baseDir);

    std::unordered_map<OttModel::Vertex, uint32_t> uniqueVertices{};

    if (!materials.empty())
    {
        materials.push_back(tinyobj::material_t());
        for (size_t i = 0; i < materials.size() - 1; i++)
        {
          LOG_INFO("material[{}].diffuse_texname = {}\n", i, materials[i].diffuse_texname);
          sceneMaterials.imageTexture_path.clear();
          auto material_path = baseDir.append(materials[i].diffuse_texname);
          sceneMaterials.imageTexture_path.push_back(material_path.string().c_str());
        }
    }
    
    OttModel::modelObject model
    {
        .startIndex  = static_cast<uint32_t>(indices.size()),
        .startVertex = static_cast<uint32_t>(vertices.size()),
        .startEdge   = static_cast<uint32_t>(edges.size()),
    };
    
    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            OttModel::Vertex vertex{};

            if (index.vertex_index >= 0)
            {
                vertex.pos =
                {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                vertex.color =
                {
                    attrib.colors[3 * index.vertex_index + 0],
                    attrib.colors[3 * index.vertex_index + 1],
                    attrib.colors[3 * index.vertex_index + 2]
                };
            }
            if (index.texcoord_index >= 0)
            {
                vertex.texCoord =
                {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };
            }
            if (index.normal_index >= 0)
            {
                vertex.normal =
                {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2],
                };
            }
            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }
            indices.push_back(uniqueVertices[vertex]);
        }
    }
    edges = OttModel::extractBoundaryEdges(indices);
    LOG_INFO("Edges Size == {}", edges.size());
    model.indexCount  = static_cast<uint32_t>(indices.size()) - model.startIndex;
    model.edgeCount   = (static_cast<uint32_t>(edges.size()) - model.startEdge);
    model.pushColorID = {Utils::random_nr(0, 1),  Utils::random_nr(0, 1), Utils::random_nr(0, 1)};
    model.textureID   = (materials.empty()) ?  0 : static_cast<uint32_t>(textureImages.size());
    models.push_back(model);
    
    LOG(DASHED_SEPARATOR);
    LOG("VERTEX COUNT: {}", vertices.size());
    LOG("model.startVertex: {}", model.startVertex);
    LOG("model.startIndex: {}",  model.startIndex);
    LOG("model.startEdge: {}",  model.startEdge);
    LOG("model.edgeCount: {}",  model.edgeCount);
    LOG("model.indexCount: {}",  model.indexCount);
    LOG("model.textureID {}",    model.textureID);
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
void OttApplication::createIndexBuffer(std::vector<uint32_t>& index, VkBuffer& index_buffer, VkDeviceMemory& index_buffer_memory)
{
    if (!vertices.empty())
    {
        VkDeviceSize bufferSize = sizeof(index[0]) * index.size();
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        appDevice.createBuffer (bufferSize,
                                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                stagingBuffer,
                                stagingBufferMemory);
        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, index.data(), (size_t) bufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

        appDevice.createBuffer (bufferSize,
                                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                index_buffer,
                                index_buffer_memory);
        appDevice.debugUtilsObjectNameInfoEXT (VK_OBJECT_TYPE_DEVICE_MEMORY, (uint64_t)index_buffer_memory, "application::VkDeviceMemory:indexBufferMemory");
        appDevice.copyBuffer(stagingBuffer, index_buffer, bufferSize);
    
        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }
}
    
//----------------------------------------------------------------------------
void OttApplication::createTextureImage(const std::filesystem::path& imagePath)
{
    int texWidth, texHeight, texChannels;
    LOG(DASHED_SEPARATOR);
    LOG_INFO("Image path: {}", imagePath.string());
    stbi_uc* pixels = stbi_load(imagePath.string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
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
void OttApplication::updateUniformBufferCamera(uint32_t currentImage, float deltaTime, int width, int height)
{        
    UniformBufferObject ubo {
        .model                 = glm::rotate(glm::mat4(1.0f), glm::radians(0.f), glm::vec3(0.0f, 0.0f, 1.0f)),
        .normalMatrix          = glm::transpose(glm::inverse(ubo.model)),
        .view                  = viewportCamera->recalculateView(deltaTime),
        .proj                  = viewportCamera->projection((float)height, (float)width),
        .viewProjectionInverse = viewportCamera->inverseProjection (
                                                viewportCamera->projection((float)height, (float)width),
                                                viewportCamera->recalculateView(deltaTime)
                                                 ),
        .cameraPos             = viewportCamera->getEyePosition(),
    };
    if (!vertices.empty()) { ubo.edgesBuffer = vkGetBufferDeviceAddress(device, &edgesBufferAddressInfo); }
    ubo.proj[1][1] *= -1;
    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}
