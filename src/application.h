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

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtx/hash.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstring>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <limits>
#include <map>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <optional>
#include <vector>

#include "camera.h"
#include "device.h"
#include "helpers.h"
#include "swapchain.h"
#include "model.h"
#include "renderer.h"
#include "window.h"
#include "utils.hxx"

#include "../stb/stb_image.h"

#include <ext/scalar_common.hpp>
#include "../external/tinyobjloader/tiny_obj_loader.h"

const int TEXTURE_ARRAY_SIZE = 1000;

const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME };

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    alignas(16) glm::mat4 viewProjectionInverse;
    alignas(16) glm::vec3 cameraPos;
};

struct PushConstantData {
    alignas(16) glm::vec3 offset;
    alignas(16) glm::vec3 color;
    alignas(4)  uint32_t  textureID;
};

//----------------------------------------------------------------------------
class OttApplication
{
//----------------------------------------------------------------------------
    public:
    //----------------------------------------------------------------------------
    
    void run();    
    GLFWwindow* getWindowhandle() const { return appwindow.getWindowhandle(); }  

    //----------------------------------------------------------------------------
    private:
    //----------------------------------------------------------------------------
    
    OttWindow appwindow = OttWindow("Ottocento Engine", 1920, 1080);
    OttDevice appDevice = OttDevice(appwindow);
    VkDevice  device    = appDevice.getDevice();
    VkPhysicalDevice physicalDevice = appDevice.getPhysicalDevice();
    OttSwapChain appSwapChain = OttSwapChain(&appDevice, &appwindow);
    OttRenderer  ottRenderer  = OttRenderer(&appDevice, &appSwapChain);

    PushConstantData push;
    std::vector<OttModel::modelObject> models;
    
    VkPipelineLayout                    pipelineLayout;
    VkDescriptorSetLayout               descriptorSetLayout;
    std::vector<VkDescriptorSetLayout>  descriptorSetLayouts;

    std::vector<VkCommandBuffer>    commandBuffers;
    uint32_t                        mipLevels;
    VkImage                         textureImage        = VK_NULL_HANDLE;
    VkDeviceMemory                  textureImageMemory  = VK_NULL_HANDLE;
    VkImageView                     textureImageView    = VK_NULL_HANDLE;
    VkSampler                       textureSampler      = VK_NULL_HANDLE;
    std::vector<VkImage>            textureImages;
    std::vector<VkImageView>        textureImageViews;
        
    std::vector<OttModel::Vertex>   vertices;
    std::vector<uint32_t>           indices;
    VkBuffer                        vertexBuffer        = VK_NULL_HANDLE;
    VkDeviceMemory                  vertexBufferMemory  = VK_NULL_HANDLE;
    VkBuffer                        indexBuffer         = VK_NULL_HANDLE;
    VkDeviceMemory                  indexBufferMemory   = VK_NULL_HANDLE;

    std::vector<VkBuffer>           uniformBuffers;
    std::vector<VkDeviceMemory>     uniformBuffersMemory;
    std::vector<void*>              uniformBuffersMapped;

    VkDescriptorPool                descriptorPool;
    std::vector<VkDescriptorSet>    descriptorSets;
    
    struct
    {
        VkPipeline grid;
        VkPipeline object;
    } graphicsPipelines;

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
    void initVulkan();
    void mainLoop();
    void drawFrame();
    void drawScene(VkCommandBuffer command_buffer);
    void cleanupTextureObjects();
    void cleanupUBO() const;
    void cleanupModelObjects() const;
    void cleanupVulkanResources();
    
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
    
    void createObjectDescriptorSetLayout();
    void createGridDescriptorSetLayout();
    void createGraphicsPipeline();
    void OttCreatePipelineLayout();
    void loadModel(std::string modelPath);
    
    void createVertexBuffer();
    void createIndexBuffer();

    // TODO: Pass these functions to a proper texel class.
    void createTextureImage(std::string imagePath);
    void createTextureImageView();
    void createTextureSampler();
    
    void createUniformBuffers();
    void createDescriptorPool();
    void createDescriptorSets();

    void updateUniformBufferCamera(uint32_t currentImage, float deltaTime, int width, int height);
};