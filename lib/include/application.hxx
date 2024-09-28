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
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <imgui.h>
#include <imconfig.h>
#include <imgui_internal.h>
// #include <imgui_impl_vulkan.h>
// #include <imgui_impl_glfw.h>
#include <imstb_rectpack.h>
#include <imstb_textedit.h>
#include <imstb_truetype.h>

#include <cstdint>
#include <vector>

#include "camera.h"
#include "helpers.h"
#include "model.h"
#include "window.h"
#include "utils.hxx"
#include "volk.h"

#include "stb_image.h"

#include <glm/ext/scalar_common.hpp>
#include "tiny_obj_loader.h"

constexpr inline int TEXTURE_ARRAY_SIZE   = 1000;
constexpr inline int MAX_FRAMES_IN_FLIGHT = 2;

const inline std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
const inline std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME };

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    alignas(16) glm::mat4 viewProjectionInverse;
    alignas(16) glm::vec3 cameraPos;
};

inline struct PushConstantData {
    alignas(16) glm::vec3 offset;
    alignas(16) glm::vec3 color;
    alignas(4)  uint32_t  textureID;
} push;

inline std::vector<OttModel::modelObject> models;

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
    
    VkInstance               instance;
    VkSurfaceKHR             surface;
    VkDebugUtilsMessengerEXT debugMessenger;

    VkPhysicalDevice         physicalDevice = VK_NULL_HANDLE;
    VkSampleCountFlagBits    msaaSamples    = VK_SAMPLE_COUNT_1_BIT;
    VkDevice                 device;
    
    VkQueue                  graphicsQueue;
    VkQueue                  presentQueue;

    VkSwapchainKHR           swapChain;
    VkFormat                 swapChainImageFormat;
    VkExtent2D               swapChainExtent;
    std::vector<VkImage>     swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    VkRenderPass             renderPass;
    VkPipelineLayout         pipelineLayout;
    
    VkDescriptorSetLayout               descriptorSetLayout;
    std::vector<VkDescriptorSetLayout>  descriptorSetLayouts;
        
    VkCommandPool                   commandPool;
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
    // Initiate GLFW window with specific parameters and sets up the window icon.
    // Windows-specific: Refresh window to darkmode.
    // AppWindow: This will assign the appwindow and  initiate the callbacks to the OttWindow GLFW wrapper.
    // This function can potentially be transfered to the window class, with a different class
    // relationship between members.
    void initWindow();
    
    //----------------------------------------------------------------------------
    // Initiates and creates Vulkan related resources.
    void initVulkan();
    
    //----------------------------------------------------------------------------
    void mainLoop();
    
    //-----------------------------------------------------------------------------
    // Cleanup function related only to elements related to our swapChain
    void cleanupSwapChain();

    //-----------------------------------------------------------------------------
    void cleanupTextureObjects();

    //-----------------------------------------------------------------------------
    void cleanupUBO();

    //-----------------------------------------------------------------------------
    void cleanupModelObjects();
    
    //----------------------------------------------------------------------------
    // Cleanup function to destroy all Vulkan allocated resources
    void cleanupVulkanResources();
    
    //----------------------------------------------------------------------------
    void createInstance();
    
    //----------------------------------------------------------------------------
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    
    //----------------------------------------------------------------------------
    void setupDebugMessenger();

    //----------------------------------------------------------------------------
    void pickPhysicalDevice();
    
    //----------------------------------------------------------------------------
    // Responsible for allocating a logical device to interface
    // with the selected Physical Device
    void createLogicalDevice();

    //----------------------------------------------------------------------------
    void createSwapChain();
    
    //-----------------------------------------------------------------------------
    void recreateSwapChain();
    
    //----------------------------------------------------------------------------
    void createImageViews();
    
    //----------------------------------------------------------------------------
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
    
    //----------------------------------------------------------------------------
    void createRenderPass();

    //----------------------------------------------------------------------------
    // A descriptor set specifies the actual buffer or image resources that will be bound to the descriptors,
    // just like a framebuffer specifies the actual image views to bind to render pass attachments.
    void createObjectDescriptorSetLayout();

    //----------------------------------------------------------------------------
    // Separate Grid DescriptorSetLayout to render the grid with a procedural shader.
    void createGridDescriptorSetLayout();
    
    //----------------------------------------------------------------------------
    // f = fixed-function stage; p = programmable stage.
    // Input Assembler (f) > Vertex Shader (p) > Tessellation (p) > Geometry Shader>
    // Rasterization (f) > Fragment Shader (p) > Colour Blending (f) > Framebuffer
    void createGraphicsPipeline();
    
    //----------------------------------------------------------------------------
    void OttCreatePipelineLayout();
    
    //----------------------------------------------------------------------------
    void createFramebuffers();    

    //----------------------------------------------------------------------------
    void createCommandPool();    

    //----------------------------------------------------------------------------
    void createColorResources();

    //----------------------------------------------------------------------------
    void createDepthResources();    

    //----------------------------------------------------------------------------
    void loadModel(std::string modelPath);

    //----------------------------------------------------------------------------
    // Buffers in Vulkan are regions of memory used for
    // storing arbitrary data that can be read by the graphics card. 
    void createVertexBuffer();

    //----------------------------------------------------------------------------
    void createIndexBuffer();    

    //----------------------------------------------------------------------------
    void createTextureImage(std::string imagePath);

    //----------------------------------------------------------------------------
    // Images are accessed through image views rather than directly, so we need to craete an
    // image view for the texture image.
    void createTextureImageView();

    //----------------------------------------------------------------------------
    // The sampler is a distinct object that provides an interface to extract colors from a texture.
    // This is different from many older APIs, which combined texture images and filtering into a single state.
    void createTextureSampler();

    //----------------------------------------------------------------------------
    void createUniformBuffers();

    //----------------------------------------------------------------------------
    void createDescriptorPool();

    //----------------------------------------------------------------------------
    // imageInfos array: Local scope only. It will take the created textureImageViews
    // + the current sampler and provide them to the descriptor writes. 
    void createDescriptorSets();

    //----------------------------------------------------------------------------
    void createCommandBuffers();

    //----------------------------------------------------------------------------
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    //----------------------------------------------------------------------------
    void createSyncObjects();    

    //----------------------------------------------------------------------------
    void updateUniformBufferCamera(uint32_t currentImage, float deltaTime, int width, int height);    

    //----------------------------------------------------------------------------
    void drawFrame();
};