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
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/hash.hpp>

#define IMGUI_IMPL_VULKAN_USE_VOLK
#include <imgui.h>
#include <imconfig.h>
#include <imgui_internal.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>
#include <imstb_rectpack.h>
#include <imstb_textedit.h>
#include <imstb_truetype.h>

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
#include "model.h"
#include "window.h"
#include "utils.hxx"

#define VOLK_IMPLEMENTATION
#include "volk.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../stb/stb_image.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <ext/scalar_common.hpp>
#include "../external/tinyobjloader/tiny_obj_loader.h"

const int TEXTURE_ARRAY_SIZE   = 1000;
const int MAX_FRAMES_IN_FLIGHT = 2;

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
} push;

std::vector<OttModel::modelObject> models;

//----------------------------------------------------------------------------
class OttApplication
{
//----------------------------------------------------------------------------
public:
//----------------------------------------------------------------------------
    void run()
    {
        initWindow();
        initVulkan();
        mainLoop();
        cleanupVulkanResources();
    }

    GLFWwindow* getWindowhandle() const { return appwindow.getWindowhandle(); }  

//----------------------------------------------------------------------------
private:
//----------------------------------------------------------------------------
    
    OttWindow appwindow = OttWindow("Ottocento Engine", 1920, 1080);
    OttDevice appDevice = OttDevice(appwindow);
    VkDevice  device    = appDevice.getDevice();
    VkPhysicalDevice physicalDevice = appDevice.getPhysicalDevice();

    VkSwapchainKHR           swapChain;
    VkFormat                 swapChainImageFormat;
    VkExtent2D               swapChainExtent;
    std::vector<VkImage>     swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    VkRenderPass             renderPass;
    VkPipelineLayout         pipelineLayout;
    
    VkDescriptorSetLayout               descriptorSetLayout;
    std::vector<VkDescriptorSetLayout>  descriptorSetLayouts;
        
    std::vector<VkFramebuffer>      swapChainFramebuffers;
    std::vector<VkCommandBuffer>    commandBuffers;
    
    VkImage                         colorImage;
    VkDeviceMemory                  colorImageMemory    = VK_NULL_HANDLE;
    VkImageView                     colorImageView;
    
    VkImage                         depthImage;
    VkDeviceMemory                  depthImageMemory    = VK_NULL_HANDLE;
    VkImageView                     depthImageView;
    
    uint32_t                        mipLevels;
    VkImage                         textureImage        = VK_NULL_HANDLE;
    VkDeviceMemory                  textureImageMemory  = VK_NULL_HANDLE;
    VkImageView                     textureImageView    = VK_NULL_HANDLE;
    VkSampler                       textureSampler      = VK_NULL_HANDLE;
    std::vector<VkImage>            textureImages;
    std::vector<VkImageView>        textureImageViews;
    
    std::vector<VkSemaphore>        imageAvailableSemaphores;
    std::vector<VkSemaphore>        renderFinishedSemaphores;
    std::vector<VkFence>            inFlightFences;
    
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
    
    uint32_t currentFrame       = 0;
    bool     framebufferResized = false;
    
    OttCamera  objectCamera;
    OttCamera* viewportCamera = &objectCamera;
    
    int windowMidPos_X, windowMidPos_Y;
    
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
    void initWindow()
    {
        viewportCamera->appwindow       = &appwindow;
        viewportCamera->windowHandle    = appwindow.getWindowhandle();
        appwindow.OnFramebufferResized  = [&](const glm::ivec2& size)
        {
            framebufferResized      = true;
            swapChainExtent.width   = size.x;
            swapChainExtent.height  = size.y;
        };
        appwindow.OnWindowRefreshed = [&]()
        {
            vkDeviceWaitIdle(device);
            //Recreate the swap chain with the new extent
            recreateSwapChain();
            updateUniformBufferCamera(currentFrame, 1, swapChainExtent.width, swapChainExtent.height);
            drawFrame();
        };
        appwindow.OnFileDropped = [&](int count, const char** paths)
        {
            vkDeviceWaitIdle(device);
            cleanupModelObjects();
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
            }
        };
        
        #ifdef _WIN32
        appwindow.ThemeRefreshDarkMode(appwindow.getWindowhandle());
        #endif
    }
    
    //----------------------------------------------------------------------------
    /** Initiates and creates Vulkan related resources. **/
    void initVulkan()
    {        
        createSwapChain();
        createImageViews();
        createRenderPass();
        createObjectDescriptorSetLayout();
        createGridDescriptorSetLayout();
        createGraphicsPipeline();
        createColorResources();
        createDepthResources();
        createFramebuffers();
        
        mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(1, 1)))) + 1;
        VkHelpers::create1x1BlankImage(textureImage, mipLevels, appDevice, textureImages, textureImageMemory);
        createTextureImageView();
        createTextureSampler();
        
        createUniformBuffers();
        createDescriptorPool();
        createDescriptorSets();
        createCommandBuffers();
        createSyncObjects();
    }
    
    //----------------------------------------------------------------------------
    void mainLoop()
    {
        while (!appwindow.windowShouldClose())
        {
            auto startTime = std::chrono::high_resolution_clock::now();
            appwindow.update();
            drawFrame();
            float deltaTime = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - startTime).count() * 0.001f * 0.001f * 0.001f;
            updateUniformBufferCamera(currentFrame, deltaTime, swapChainExtent.width, swapChainExtent.height);
        }
        vkDeviceWaitIdle(device);
    }
    
    //-----------------------------------------------------------------------------
    // Cleanup function related only to elements related to our swapChain
    void cleanupSwapChain()
    {
        vkDestroyImageView (device, depthImageView,   nullptr);
        vkDestroyImage     (device, depthImage,       nullptr);
        vkFreeMemory       (device, depthImageMemory, nullptr);

        vkDestroyImageView (device, colorImageView,   nullptr);
        vkDestroyImage     (device, colorImage,       nullptr);
        vkFreeMemory       (device, colorImageMemory, nullptr);
        
        for (size_t i = 0; i < swapChainFramebuffers.size(); i++) 
            vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
        for (size_t i = 0; i < swapChainImageViews.size(); i++)
            vkDestroyImageView(device, swapChainImageViews[i], nullptr);
        vkDestroySwapchainKHR(device, swapChain, nullptr);
    }

    //-----------------------------------------------------------------------------
    void cleanupTextureObjects()
    {
        std::cout << "cleanupTextureObjects() start :::::" << std::endl;
        for (uint8_t i = 0; i < textureImageViews.size() - 1; i++)
        {
            std::cout << "Texture Image views size: " << textureImageViews.size() << std::endl;
            if (textureImageViews[i] != VK_NULL_HANDLE) { vkDestroyImageView (device, textureImageViews[i], nullptr); }
            if (textureImages[i]     != VK_NULL_HANDLE) { vkDestroyImage     (device, textureImages[i],     nullptr); }
        }
        textureImageViews.clear();
        if (textureImageView != VK_NULL_HANDLE) { vkDestroyImageView (device, textureImageView, nullptr); }
        if (textureImage != VK_NULL_HANDLE)     { vkDestroyImage     (device, textureImage,     nullptr); }
        if (textureImageMemory != VK_NULL_HANDLE) { vkFreeMemory     (device, textureImageMemory, nullptr); }
    }

    //-----------------------------------------------------------------------------
    void cleanupUBO()
    {
        std::cout << "cleanupUBO() start :::::" << std::endl;
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
    void cleanupModelObjects()
    {
        std::cout << "cleanupModelObjects start :::::" << std::endl;
        if (indexBuffer != VK_NULL_HANDLE)       { vkDestroyBuffer  (device, indexBuffer,       nullptr); }
        if (indexBufferMemory != VK_NULL_HANDLE) { vkFreeMemory     (device, indexBufferMemory, nullptr); }
        
        if (vertexBuffer != VK_NULL_HANDLE)       { vkDestroyBuffer  (device, vertexBuffer,       nullptr); }
        if (vertexBufferMemory != VK_NULL_HANDLE) { vkFreeMemory     (device, vertexBufferMemory, nullptr); }
    }
    
    //----------------------------------------------------------------------------
    // Cleanup function to destroy all Vulkan allocated resources
    void cleanupVulkanResources()
    {
        cleanupSwapChain();
        cleanupTextureObjects();
        if (textureSampler != VK_NULL_HANDLE)   { vkDestroySampler   (device, textureSampler,   nullptr); }
        cleanupUBO();
        cleanupModelObjects();
        
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }
                
        vkDestroyPipeline(device, graphicsPipelines.object, nullptr);
        vkDestroyPipeline(device, graphicsPipelines.grid, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        vkDestroyRenderPass(device, renderPass, nullptr);
    }
    
    //----------------------------------------------------------------------------
    void createSwapChain()
    {
        SwapChainSupportDetails swapChainSupport = appDevice.querySwapChainSupport();

        VkSurfaceFormatKHR surfaceFormat = VkHelpers::chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR   presentMode   = VkHelpers::chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D         extent        = VkHelpers::chooseSwapExtent(swapChainSupport.capabilities, &appwindow);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
            imageCount = swapChainSupport.capabilities.maxImageCount;

        VkSwapchainCreateInfoKHR createInfo {
                                .sType              = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                                .surface            = appDevice.getSurface(),
                                .minImageCount      = imageCount,
                                .imageFormat        = surfaceFormat.format,
                                .imageColorSpace    = surfaceFormat.colorSpace,
                                .imageExtent        = extent,
                                .imageArrayLayers   = 1,
                                .imageUsage         = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                .preTransform       = swapChainSupport.capabilities.currentTransform,
                                .compositeAlpha     = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                                .presentMode        = presentMode,
                                .clipped            = VK_TRUE,
        };
        
        QueueFamilyIndices indices = appDevice.findQueueFamilies();
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        if (indices.graphicsFamily != indices.presentFamily)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0; // Optional
            createInfo.pQueueFamilyIndices = nullptr; // Optional
        }
        
        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
            throw std::runtime_error("failed to create swap chain!");
        
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
    }
    
    //-----------------------------------------------------------------------------
    void recreateSwapChain()
    {
        int width = appwindow.getFrameBufferSize().x;
        int height = appwindow.getFrameBufferSize().y;
        while (width == 0 || height == 0)
        {
            width = appwindow.getFrameBufferSize().x; height = appwindow.getFrameBufferSize().y;
            appwindow.waitEvents();
        }
        vkDeviceWaitIdle(device);
        
        cleanupSwapChain();
        
        createSwapChain();
        createImageViews();
        createColorResources();
        createDepthResources();
        createFramebuffers();
    }
    
    //----------------------------------------------------------------------------
    void createImageViews()
    {
        swapChainImageViews.resize(swapChainImages.size());
        for (uint32_t i = 0; i < swapChainImages.size(); i++)
            swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
    
    //----------------------------------------------------------------------------
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
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
    void createRenderPass()
    {
        VkAttachmentDescription colorAttachment {
                                .format         = swapChainImageFormat,
                                .samples        = appDevice.getMSAASamples(),
                                .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
                                .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
                                .finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        };

        VkAttachmentDescription depthAttachment {
                                .format         = appDevice.findDepthFormat(),
                                .samples        = appDevice.getMSAASamples(),
                                .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                .storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
                                .finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        };

        VkAttachmentDescription colorAttachmentResolve {
                                .format         = swapChainImageFormat,
                                .samples        = VK_SAMPLE_COUNT_1_BIT,
                                .loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
                                .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
                                .finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        };
        std::array<VkAttachmentDescription, 3> attachments = {colorAttachment, depthAttachment, colorAttachmentResolve};
        
        VkAttachmentReference colorAttachmentRef {
                              .attachment = 0,
                              .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        };

        VkAttachmentReference depthAttachmentRef {
                              .attachment = 1,
                              .layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        };

        VkAttachmentReference colorAttachmentResolveRef {
                              .attachment = 2,
                              .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        };

        VkSubpassDescription subpass {
                            .pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
                            .colorAttachmentCount    = 1,
                            .pColorAttachments       = &colorAttachmentRef,
                            .pResolveAttachments     = &colorAttachmentResolveRef,
                            .pDepthStencilAttachment = &depthAttachmentRef,
        };

        VkSubpassDependency dependency {
                            .srcSubpass    = VK_SUBPASS_EXTERNAL,
                            .dstSubpass    = 0,
                            .srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT  | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                            .dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                            .srcAccessMask = 0,
                            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        };

        VkRenderPassCreateInfo renderPassInfo {
                            .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                            .attachmentCount = static_cast<uint32_t>(attachments.size()),
                            .pAttachments    = attachments.data(),
                            .subpassCount    = 1,
                            .pSubpasses      = &subpass,
                            .dependencyCount = 1,
                            .pDependencies   = &dependency,
        };

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
            throw std::runtime_error("failed to create render pass!");
        std::cout << "Render Pass Created" << std::endl;
    }

    //----------------------------------------------------------------------------
    // A descriptor set specifies the actual buffer or image resources that will be bound to the descriptors,
    // just like a framebuffer specifies the actual image views to bind to render pass attachments.
    void createObjectDescriptorSetLayout()
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
                                     .descriptorCount    = TEXTURE_ARRAY_SIZE,
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
        std::cout << "descriptorSetLayouts Size: " << descriptorSetLayouts.size() << std::endl;
    }

    //----------------------------------------------------------------------------
    // Separate Grid DescriptorSetLayout to render the grid with a procedural shader.
    void createGridDescriptorSetLayout()
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
        std::cout << "descriptorSetLayouts Size: " << descriptorSetLayouts.size() << std::endl;
    }
    
    //----------------------------------------------------------------------------
    /** f = fixed-function stage; p = programmable stage.
     *  Input Assembler (f) > Vertex Shader (p) > Tessellation (p) > Geometry Shader >
     *  Rasterization (f) > Fragment Shader (p) > Colour Blending (f) > Framebuffer **/
    void createGraphicsPipeline()
    {
        auto vertShaderCode     = Utils::readFile("src/shaders/vert.spv");
        auto fragShaderCode     = Utils::readFile("src/shaders/frag.spv");
        auto gridVertShaderCode = Utils::readFile("src/shaders/gridVert.spv");
        auto gridFragShaderCode = Utils::readFile("src/shaders/gridFrag.spv");

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
                                        // .blendConstants[0] = 0.0f, // Optional
                                        // .blendConstants[1] = 0.0f, // Optional
                                        // .blendConstants[2] = 0.0f, // Optional
                                        // .blendConstants[3] = 0.0f, // Optional
        };

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
                                        .renderPass          = renderPass,
                                        .subpass             = 0,
                                        .basePipelineHandle  = VK_NULL_HANDLE,
        };
        
        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE,
                                    1,
                                    &pipelineInfo,
                                    nullptr,
                                    &graphicsPipelines.object) != VK_SUCCESS)
            throw std::runtime_error("failed to create graphics pipeline!");
        else { std::cout << "Object Pipeline Created" << std::endl; }

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
        
        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE,
                                   1,
                                   &pipelineInfo,
                                   nullptr,
                                   &graphicsPipelines.grid) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create graphics pipeline!");
        }
        else { std::cout << "Grid Pipeline Created" << std::endl; }
        
        // Shader modules are just a thin wrapper around the shader
        // bytecode that we've previously loaded from a file and the functions defined in it.
        // That means that we're allowed to destroy the shader modules again
        // as soon as pipeline creation is finished
        vkDestroyShaderModule(device, fragShaderModule, nullptr);
        vkDestroyShaderModule(device, vertShaderModule, nullptr);
        vkDestroyShaderModule(device, gridVertShaderModule, nullptr);
        vkDestroyShaderModule(device, gridFragShaderModule, nullptr);
    }
    
    //----------------------------------------------------------------------------
    void OttCreatePipelineLayout()
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
    void createFramebuffers()
    {
        swapChainFramebuffers.resize(swapChainImageViews.size());
        for (size_t i = 0; i < swapChainImageViews.size(); i++)
        {
            std::array<VkImageView, 3> attachments = {
                colorImageView,
                depthImageView,
                swapChainImageViews[i],
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
                throw std::runtime_error("failed to create framebuffer!");
        }
    }
        
    //----------------------------------------------------------------------------
    void createColorResources()
    {
        const VkFormat colorFormat = swapChainImageFormat;
        VkHelpers::createImage(swapChainExtent.width, swapChainExtent.height, 1, appDevice.getMSAASamples(),
                                colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage, colorImageMemory, appDevice);
        appDevice.debugUtilsObjectNameInfoEXT(VK_OBJECT_TYPE_DEVICE_MEMORY, (uint64_t)colorImageMemory, "application::VkDeviceMemory:colorImageMemory");
        colorImageView = createImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }

    //----------------------------------------------------------------------------
    void createDepthResources()
    {
        VkFormat depthFormat = appDevice.findDepthFormat();
        
        VkHelpers::createImage(swapChainExtent.width, swapChainExtent.height, 1, appDevice.getMSAASamples(), depthFormat,
                                VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory,
                                appDevice);
        depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
        VkHelpers::transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1,
                                         appDevice);
        appDevice.debugUtilsObjectNameInfoEXT(VK_OBJECT_TYPE_DEVICE_MEMORY, (uint64_t)depthImageMemory, "application::VkDeviceMemory:depthImageMemory");
    }
    
    //----------------------------------------------------------------------------
    void loadModel(std::string modelPath)
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;
        std::string baseDir = Utils::GetBaseDir(modelPath) + "\\";
                
        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str(), Utils::GetBaseDir(modelPath).c_str()))
            throw std::runtime_error(warn + err);

        std::cout << "----------------------------------------"   << std::endl;
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
        
        std::cout << "----------------------------------------"   << std::endl;
        std::cout << "VERTEX COUNT: "       << vertices.size()    << std::endl;
        std::cout << "model.startVertex: "  << model.startVertex  << std::endl;
        std::cout << "model.startIndex: "   << model.startIndex   << std::endl;
        std::cout << "model.indexCount: "   << model.indexCount   << std::endl;
        std::cout << "model.textureID "     << model.textureID    << std::endl;
        std::cout << "----------------------------------------"   << std::endl;
    }

    //----------------------------------------------------------------------------
    // Buffers in Vulkan are regions of memory used for
    // storing arbitrary data that can be read by the graphics card. 
    void createVertexBuffer()
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
            appDevice.debugUtilsObjectNameInfoEXT (VK_OBJECT_TYPE_DEVICE_MEMORY, (uint64_t)vertexBufferMemory, "application::VkDeviceMemory:vertexBufferMemory");
            appDevice.debugUtilsObjectNameInfoEXT (VK_OBJECT_TYPE_BUFFER, (uint64_t)vertexBuffer, "application::VkBuffer:vertexBuffer");
            appDevice.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

            vkDestroyBuffer(device, stagingBuffer, nullptr);
            vkFreeMemory(device, stagingBufferMemory, nullptr);
        }
    }

    //----------------------------------------------------------------------------
    void createIndexBuffer()
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
    void createTextureImage(std::string imagePath)
    {
        int texWidth, texHeight, texChannels;
        std::cout << "----------------------------------------"   << std::endl;
        std::cout << "Image path "          << imagePath          << std::endl;
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

        VkHelpers::createImage (texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                                VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory, appDevice
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
    // Images are accessed through image views rather than directly, so we need to craete an
    // image view for the texture image.
    void createTextureImageView()
    {
        textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
        textureImageViews.push_back(textureImageView);
    }

    //----------------------------------------------------------------------------
    // The sampler is a distinct object that provides an interface to extract colors from a texture.
    // This is different from many older APIs, which combined texture images and filtering into a single state.
    void createTextureSampler()
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
    void createUniformBuffers()
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
            appDevice.debugUtilsObjectNameInfoEXT(VK_OBJECT_TYPE_DEVICE_MEMORY, (uint64_t) uniformBuffersMemory[i], "application::VkDeviceMemory:uniformBuffersMemory");
        }
    }

    //----------------------------------------------------------------------------
    void createDescriptorPool()
    {
        std::array<VkDescriptorPoolSize, 4> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; 
        poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[2].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[3].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

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
    // imageInfos array: Local scope only. It will take the created textureImageViews
    // + the current sampler and provide them to the descriptor writes. 
    void createDescriptorSets()
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
        if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
            throw std::runtime_error("failed to allocate descriptor sets!");

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
        }
    }

    //----------------------------------------------------------------------------
    void createCommandBuffers()
    {
        commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        
        VkCommandBufferAllocateInfo allocInfo {
                                    .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                                    .commandPool        = appDevice.getCommandPool(),
                                    .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                    .commandBufferCount = (uint32_t) commandBuffers.size(),
        };
        
        if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
            throw std::runtime_error("failed to allocate command buffers!");
    }

    //----------------------------------------------------------------------------
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
            throw std::runtime_error("failed to begin recording command buffer!");

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapChainExtent;

        // The order of clearValues should be identical to the order of attachments.
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {{0.015f, 0.015f, 0.015f, 1.0f}};
        clearValues[1].depthStencil = {1.0f, 0};

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        if (static_cast<float>(swapChainExtent.width) > 0.0f && static_cast<float>(swapChainExtent.height) > 0.0f)
        {
            vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            VkViewport viewport{};
            viewport.x          = 0.0f;
            viewport.y          = 0.0f;
            viewport.width      = static_cast<float>(swapChainExtent.width);
            viewport.height     = static_cast<float>(swapChainExtent.height);
            viewport.minDepth   = 0.0f;
            viewport.maxDepth   = 1.0f;
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        
            VkRect2D scissor{};
            scissor.offset      = {0, 0};
            scissor.extent      = swapChainExtent;
            vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                                    0, 1, &descriptorSets[currentFrame], 0, nullptr);

            if (!vertices.empty() && !indices.empty())
            {
                vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelines.object);
                VkBuffer vertexBuffers[] = {vertexBuffer};
                VkDeviceSize offsets[] = {0};
                vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
                vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
                for (auto& m : models)
                {
                    push.color      = m.pushColorID;
                    push.textureID  = m.textureID;
                    vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantData), &push);
                    vkCmdDrawIndexed(commandBuffer, m.indexCount, 1, m.startIndex, 0, 0);
                }
            }
        
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelines.grid);
            vkCmdDraw(commandBuffer, 6, 1, 0, 0);
            vkCmdEndRenderPass(commandBuffer);
        }
        
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
            throw std::runtime_error("failed to record command buffer!");
    }

    //----------------------------------------------------------------------------
    void createSyncObjects()
    {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        
        VkSemaphoreCreateInfo semaphoreInfo {
                              .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };
        
        VkFenceCreateInfo fenceInfo {
                          .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                          .flags = VK_FENCE_CREATE_SIGNALED_BIT,
        };
        
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i])                   != VK_SUCCESS
                )
                    {
                        throw std::runtime_error("failed to create semaphores!");
                    }
            appDevice.debugUtilsObjectNameInfoEXT (VK_OBJECT_TYPE_SEMAPHORE, (uint64_t)imageAvailableSemaphores[i], "application::VkSemaphore:imageAvailableSemaphore");
            appDevice.debugUtilsObjectNameInfoEXT (VK_OBJECT_TYPE_SEMAPHORE, (uint64_t)renderFinishedSemaphores[i], "application::VkSemaphore:renderFinishedSemaphores");
        }
    }
    
    //----------------------------------------------------------------------------
    void updateUniformBufferCamera(uint32_t currentImage, float deltaTime, int width, int height)
    {        
        UniformBufferObject ubo {
                            .model                 = glm::rotate(glm::mat4(1.0f), glm::radians(270.f), glm::vec3(0.0f, 0.0f, 1.0f)),
                            .view                  = viewportCamera->recalculateView(deltaTime),
                            .proj                  = viewportCamera->perspectiveProjection(width / (float)height),
                            .viewProjectionInverse = viewportCamera->inverseProjection(ubo.proj, ubo.view),
                            .cameraPos             = viewportCamera->getEyePosition(),
        };
        ubo.proj[1][1] *= -1,
        memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
    }
    
    //----------------------------------------------------------------------------
    void drawFrame()
    {
        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
        
        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        // Only reset the fence if we are submitting work
        vkResetFences(device, 1, &inFlightFences[currentFrame]);
        vkResetCommandBuffer(commandBuffers[currentFrame], 0);
        recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

        VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
        
        if (vkQueueSubmit(appDevice.getGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) 
            throw std::runtime_error("failed to submit draw command buffer!");
        
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {swapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr; // Optional

        result = vkQueuePresentKHR(appDevice.getPresentQueue(), &presentInfo);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
        {
            framebufferResized = false;
            recreateSwapChain();
        }
        else if (result != VK_SUCCESS)
            throw std::runtime_error("failed to present swap chain image!");
        
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }
};
