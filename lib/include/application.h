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

#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <cstdint>
#include <filesystem>
#include <vector>

#include "camera.h"
#include "device.h"
#include "descriptor.h"
#include "swapchain.h"
#include "model.h"
#include "pipeline.h"
#include "renderer.h"
#include "window.h"

const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME };

//----------------------------------------------------------------------------
class OttApplication
{
//----------------------------------------------------------------------------
public:
//----------------------------------------------------------------------------
    
    void run(const std::filesystem::path&);    
    GLFWwindow* getWindowhandle() const { return appwindow.getWindowhandle(); }  

//----------------------------------------------------------------------------
private:
//----------------------------------------------------------------------------
    
    OttWindow appwindow = OttWindow("Ottocento Engine", 1920, 1080);
    OttDevice appDevice = OttDevice(appwindow);
    VkDevice  device    = appDevice.getDevice();
    VkPhysicalDevice physicalDevice = appDevice.getPhysicalDevice();
    OttSwapChain appSwapChain = OttSwapChain (&appDevice, &appwindow);
    OttRenderer  ottRenderer  = OttRenderer  (&appDevice, &appSwapChain);
    OttPipeline  appPipeline  = OttPipeline  (&appDevice, &appSwapChain);

    PushConstantData push;
    std::vector<OttModel::modelObject> models;
    
    VkDescriptorSetLayout bindlessDescSetLayout = OttDescriptor::createBindlessDescriptorSetLayout(device, appDevice);
    VkDescriptorSet  bindlessDescriptorSet;
    VkDescriptorPool descriptorPool;
    std::unordered_map<std::string, VkDescriptorSet> descriptorSets;
    
    std::vector<VkCommandBuffer>    commandBuffers;
    uint32_t                        mipLevels           = static_cast<uint32_t>(std::floor(std::log2(std::max(1, 1)))) + 1;
    VkImage                         textureImage        = VK_NULL_HANDLE;
    std::vector<VkDeviceMemory>     textureImageMemory  = { VK_NULL_HANDLE };
    VkImageView                     textureImageView    = VK_NULL_HANDLE;
    VkSampler                       textureSampler      = VK_NULL_HANDLE;
    std::vector<VkImage>            textureImages;
    std::vector<VkImageView>        textureImageViews;
        
    std::vector<OttModel::Vertex>   vertices;
    std::vector<uint32_t>           indices;
    std::vector<uint32_t>           edges;
    
    VkBuffer                        vertexBuffer        = VK_NULL_HANDLE;
    VkDeviceMemory                  vertexBufferMemory  = VK_NULL_HANDLE;
    VkBuffer                        indexBuffer         = VK_NULL_HANDLE;
    VkDeviceMemory                  indexBufferMemory   = VK_NULL_HANDLE;
    VkBuffer                        edgesBuffer         = VK_NULL_HANDLE;
    VkDeviceMemory                  edgesBufferMemory   = VK_NULL_HANDLE;
    VkBufferDeviceAddressInfo       edgesBufferAddressInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .pNext = nullptr,
        .buffer = edgesBuffer,
    };

    std::vector<VkBuffer>           uniformBuffers;
    std::vector<VkDeviceMemory>     uniformBuffersMemory;
    std::vector<void*>              uniformBuffersMapped;
    
    struct
    {
        std::vector<std::string> imageTexture_path;
    } sceneMaterials;
        
    OttCamera  objectCamera;
    OttCamera* viewportCamera = &objectCamera;
    int windowMidPos_X, windowMidPos_Y;
    
    
    //----------------------------------------------------------------------------
    // Main Pipeline functions
    //----------------------------------------------------------------------------
    
    void initWindow();
    void initVulkan(const std::filesystem::path& shader_dir);
    void mainLoop();
    void drawFrame();
    void drawScene(VkCommandBuffer command_buffer);
    void cleanupTextureObjects();
    void cleanupUBO() const;
    void cleanupModelObjects() const;
    void cleanupVulkanResources();
    
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevelCount);
    
    void loadModel(std::filesystem::path const& modelPath);
    
    void createVertexBuffer();
    void createIndexBuffer(std::vector<uint32_t>& indices, VkBuffer& index_buffer, VkDeviceMemory& index_buffer_memory);

    // TODO: Pass these functions to a proper texel class.
    void createTextureImage(const std::filesystem::path& imagePath);
    void createTextureImageView();
    void createTextureSampler();
    
    void createUniformBuffers();
    void updateUniformBufferCamera(uint32_t currentImage, float deltaTime, float width, float height);
};
