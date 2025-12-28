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

#include <fmt/base.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include "device.h"
#include "macros.h"
#include <cstring>
#include <set>
#include <map>
#include <volk.h>

namespace
{
//----------------------------------------------------------------------------
/** Sets the Validation layers debugCallbacks.
 *  This will show the Vulkan validation messages while in Debug Mode on our application.**/
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                            VkDebugUtilsMessageTypeFlagsEXT messageType,
                                            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                            void* pUserData)
{
    fmt::println(stderr, "validation layer: {}", pCallbackData->pMessage);
    return VK_FALSE;
}
} // anonymous namespace

//----------------------------------------------------------------------------
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

//----------------------------------------------------------------------------
void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {
        .sType            = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity  = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType      = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback  = debugCallback,
    };
}

//----------------------------------------------------------------------------
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

//----------------------------------------------------------------------------
// Device Specific function definitions --------------------------------------
//----------------------------------------------------------------------------
// Public Functions
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
/** Default OttDevice constructor.
 * \param window: the device needs a window properly created in advance
 * to be instantiated. **/
OttDevice::OttDevice(OttWindow& window)
{
    createInstance();
    volkLoadInstance(instance);
    setupDebugMessenger();
    createWindowSurface(window);
    pickPhysicalDevice();
    createLogicalDevice();
    volkLoadDevice(device);
    createCommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
}

//----------------------------------------------------------------------------
/** Default OttDevice destructor. **/
OttDevice::~OttDevice()
{
    vkDestroyCommandPool(device, commandPool, nullptr);
    vkDestroyDevice(device, nullptr);
    if (enableValidationLayers)
        ::DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
}

//----------------------------------------------------------------------------
/** Temporary command buffer is allocated from the
 *  command pool with the VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT flag, which indicates that the command buffer
 *  will only be submitted once.
 *  The command buffer is then begun, allowing for recording of commands. **/
VkCommandBuffer OttDevice::beginSingleTimeCommands() const
{
    const VkCommandBufferAllocateInfo allocInfo {
                                        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                                        .commandPool        = commandPool,
                                        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                        .commandBufferCount = 1,
    };

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    const VkCommandBufferBeginInfo beginInfo {
                                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    return commandBuffer;
}

//----------------------------------------------------------------------------
/** Once the necessary commands have been recorded into the temporary command buffer,
 *  endSingleTimeCommands is called to end the recording of
 *  commands, submit the command buffer for execution, and wait for the command to finish before
 *  cleaning up and freeing the temporary command buffer. **/
void OttDevice::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    const VkSubmitInfo submitInfo {
                        .sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                        .commandBufferCount = 1,
                        .pCommandBuffers    = &commandBuffer,
    };

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);
    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

//----------------------------------------------------------------------------
/** This function is used to create a buffer in Vulkan. It involves the following steps:
 *  Creating a buffer object, allocating memory for the buffer, binding the buffer to the allocated memory,
 *  and finally mapping the buffer memory if necessary. **/
void OttDevice::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags propertiesFlags, VkBuffer& buffer, VkDeviceMemory &bufferMemory)
{
    const VkBufferCreateInfo bufferInfo {
                            .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                            .size        = size,
                            .usage       = usage,
                            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    VkResult result = vkCreateBuffer(device, &bufferInfo, nullptr, &buffer);
    if (result != VK_SUCCESS)
    {
        log_t<error>("vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) returned {}", static_cast<int>(result));
        throw std::runtime_error("Failed to create buffer.");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    const VkMemoryAllocateInfo allocInfo {
                               .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                               .allocationSize  = memRequirements.size,
                               .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, propertiesFlags),
    };

    if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate buffer memory!");
    vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

//----------------------------------------------------------------------------
/** This function typically involves recording a command to copy the data from a source buffer to a destination buffer.
 *  It is useful for scenarios where you need to transfer data between buffers. **/
void OttDevice::copyBuffer(VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize size)
{
    const VkCommandBuffer commandBuffer = beginSingleTimeCommands();
        VkBufferCopy copyRegion {
                     .size = size,
        };
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    endSingleTimeCommands(commandBuffer);
}

//----------------------------------------------------------------------------
/** This function is used to copy data from a buffer to an image in Vulkan.
 *  It involves recording a command to copy the data from a source buffer to a destination image.
 *  This is often used when you need to upload texture data stored in a buffer to an image for rendering purposes. **/
void OttDevice::copyBufferToImage(VkBuffer& buffer, VkImage& image, uint32_t width, uint32_t height)
{
    const VkCommandBuffer commandBuffer = beginSingleTimeCommands();
        const VkBufferImageCopy region {
                                  .bufferOffset      = 0,
                                  .bufferRowLength   = 0,
                                  .bufferImageHeight = 0,

                                  .imageSubresource
                                 {
                                      .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                                      .mipLevel       = 0,
                                      .baseArrayLayer = 0,
                                      .layerCount     = 1,
                                  },
                    
                                  .imageOffset = {0, 0, 0},
                                  .imageExtent = { width, height, 1 },
        };
        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    endSingleTimeCommands(commandBuffer);
}

//----------------------------------------------------------------------------
/** Dedicated function to fill the VkDebugUtilsObjectNameInfoEXT struct and pass it to the
 *  vkSetDebugUtilsObjectNameEXT function. It's basically a small wrapper for convenience. **/
void OttDevice::debugUtilsObjectNameInfoEXT(const VkObjectType objType, const uint64_t objHandle, const std::string objName) const
{
    if (enableValidationLayers == true)
    {
        const VkDebugUtilsObjectNameInfoEXT debugNameInfo {
            .sType        = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext        = nullptr,
            .objectType   = objType,
            .objectHandle = objHandle,
            .pObjectName  = objName.c_str(),
        };
        if (vkSetDebugUtilsObjectNameEXT(device, &debugNameInfo) != VK_SUCCESS)
            throw std::runtime_error("Failed to load Object Name Extension");
    }
}

//----------------------------------------------------------------------------
// Private Functions
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
/** The instance is the connection between the application and the
 *  Vulkan library and creating it involves specifying some details about the application to the driver **/
void OttDevice::createInstance()
{
    if (volkInitialize() != VK_SUCCESS) { return; }
    if (enableValidationLayers && !checkValidationLayerSupport(validationLayers)) 
        throw std::runtime_error("validation layers requested, but not available!");

    VkApplicationInfo   appInfo {
                        .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                        .pApplicationName   = "OttocentoEngine",
                        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
                        .pEngineName        = "No Engine",
                        .engineVersion      = VK_MAKE_VERSION(1, 0, 0),
                        .apiVersion         = VK_API_VERSION_1_3,
    };

    auto extensions = getRequiredExtensions();
    VkInstanceCreateInfo createInfo {
                        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                        .pApplicationInfo = &appInfo,
                        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
                        .ppEnabledExtensionNames = extensions.data(),
    };

    #ifdef __APPLE__
    extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    ++createInfo.enabledExtensionCount;
    #endif

    for (const auto& extension : extensions) {
        log_t<info>("Extensions List:: {}", extension);
    }

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        ::populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }
    
    const VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    if (result != VK_SUCCESS)
    {
        log_t<error>("vkCreateInstance returned: {}", static_cast<int>(result));
        throw std::runtime_error("failed to create instance!");
    }
    log_t<info>("Vulkan Instance Created");
}

//----------------------------------------------------------------------------
void OttDevice::setupDebugMessenger()
{
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    ::populateDebugMessengerCreateInfo(createInfo);

    if (::CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
        throw std::runtime_error("failed to set up debug messenger!");
}

//----------------------------------------------------------------------------
/** Wrapper for the glfwCreateWindowSurface function.
 *  More details can be provided by the original GLFW function.
 *  \param window: We need the window properly created in advance
 *  to call the aforementioned GLFW function. **/
void OttDevice::createWindowSurface(const OttWindow& window)
{
    if (glfwCreateWindowSurface(instance, window.getWindowhandle(), nullptr, &surface) != VK_SUCCESS)
        throw std::runtime_error("failed to create window surface!");
}

//----------------------------------------------------------------------------
/** Rates and pick a GPU for usage. **/
void OttDevice::pickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        
    if (deviceCount == 0) 
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
        
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
        
    std::multimap<int, VkPhysicalDevice> candidates;

    for (const auto& physical_device : devices)
    {
        int score = rateDeviceSuitability(physical_device);
        candidates.insert(std::make_pair(score, physical_device));
    }
        
    if (candidates.rbegin()->first > 0 && isDeviceSuitable(candidates.rbegin()->second, deviceExtensions))
    {
        if ((physicalDevice = candidates.rbegin()->second))
        {
            msaaSamples = getMaxUsableSampleCount();
            physical_maxDescriptorSampledImageCount = getMaxDescriptorSampleCount();
            log_t<info>("GPU is properly scored and suitable for usage.");
            log_t<info>("Max Usable Sample Count: {} xMSAA", unsigned(msaaSamples));
            log_t<info>("maxDescriptorSampledImageCount: {}", unsigned(physical_maxDescriptorSampledImageCount));
        }
    }
        
    if (physicalDevice == VK_NULL_HANDLE)
        throw std::runtime_error("failed to find a suitable GPU!");
}

//----------------------------------------------------------------------------
/** Responsible for allocating a logical device to interface
 *  with the selected Physical Device. **/
void OttDevice::createLogicalDevice()
{
    using enum fmt::color;

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo {
                                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                                .queueFamilyIndex = indices.graphicsFamily.value(),
                                .queueCount = 1,
                                .pQueuePriorities = &queuePriority,
        };
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceVulkan12Features physicalDeviceVulkan12Features {
                                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
                                .shaderSampledImageArrayNonUniformIndexing     = VK_TRUE,
                                .descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE,
                                .descriptorBindingSampledImageUpdateAfterBind  = VK_TRUE,
                                .descriptorBindingPartiallyBound               = VK_TRUE,
                                .descriptorBindingVariableDescriptorCount      = VK_TRUE,
                                .runtimeDescriptorArray                        = VK_TRUE,
                                .bufferDeviceAddress                           = VK_TRUE
    };
    
    VkPhysicalDeviceFeatures2 deviceFeatures {
                                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
                                .pNext = &physicalDeviceVulkan12Features,
                                .features = {
                                                .sampleRateShading = VK_TRUE,
                                                .fillModeNonSolid  = VK_TRUE,
                                                .samplerAnisotropy = VK_TRUE,
                                }
    };

    VkDeviceCreateInfo createInfo {
                                .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                                .pNext = &deviceFeatures,
                                .queueCreateInfoCount    = static_cast<uint32_t>(queueCreateInfos.size()),
                                .pQueueCreateInfos       = queueCreateInfos.data(),
                                .enabledExtensionCount   = static_cast<uint32_t>(deviceExtensions.size()),
                                .ppEnabledExtensionNames = deviceExtensions.data(),
    };

    /** "Previous implementations of Vulkan made a distinction between instance and device specific
     *  validation layers, but this is no longer the case.
     *  However, it is still a good idea to set them anyway to be compatible with older implementations:" **/
    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else 
        createInfo.enabledLayerCount = 0;

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
        throw std::runtime_error("failed to create logical device!");
    
    debugUtilsObjectNameInfoEXT (VK_OBJECT_TYPE_PHYSICAL_DEVICE, reinterpret_cast<uint64_t>(physicalDevice), color_str<red>(" OttDevice::physicalDevice "));
    debugUtilsObjectNameInfoEXT (VK_OBJECT_TYPE_DEVICE, reinterpret_cast<uint64_t>(device), color_str<red>(" OttDevice::device "));
    debugUtilsObjectNameInfoEXT (VK_OBJECT_TYPE_INSTANCE, reinterpret_cast<uint64_t>(instance), color_str<red>(" OttDevice::VkInstance::instance "));
    
    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
    
    log_t<info>("Logical Device Successfully created");
}

//----------------------------------------------------------------------------
/** Command pools manage the memory that is used to store the buffers and command buffers are allocated from them.
 *  \param flags: There are two possible flags for command pools:
     - VK_COMMAND_POOL_CREATE_TRANSIENT_BIT:
       Hint that command buffers are rerecorded with new commands very often (may change memory allocation behavior)
     - VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT:
       Allow command buffers to be rerecorded individually, without this flag they all have to be reset together **/
void OttDevice::createCommandPool(VkCommandPoolCreateFlags flags)
{
    const QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);
    const VkCommandPoolCreateInfo poolInfo {
                                  .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                                  .flags            = flags,
                                  .queueFamilyIndex = queueFamilyIndices.graphicsFamily.value(),
    };

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
        throw std::runtime_error("failed to create command pool!");
    log_t<info>("CommandPool Created");
}

//----------------------------------------------------------------------------
// Helper Functions
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
/** Function that will return the required list of extensions
 *  based on whether validation layers are enabled or not. **/
std::vector<const char*> OttDevice::getRequiredExtensions() const
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers)
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    return extensions;
}


//----------------------------------------------------------------------------
/** Polls the active GPU to get the maximum usable sample count for Multisampling Anti-aliasing. **/
VkSampleCountFlagBits OttDevice::getMaxUsableSampleCount() const
{
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

    VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
    if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
    if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
    if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
    if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
    if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }
    return VK_SAMPLE_COUNT_1_BIT;
}

//----------------------------------------------------------------------------
/** Polls the active GPU to get the maximum usable sample count for Descriptors. **/
uint32_t OttDevice::getMaxDescriptorSampleCount() const
{
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
    return physicalDeviceProperties.limits.maxPerStageDescriptorSampledImages;
}

//----------------------------------------------------------------------------
/** Queries if the physical device and the window surface both support the
 * swap chain present mode. **/
SwapChainSupportDetails OttDevice::querySwapChainSupport(VkPhysicalDevice physical_device)
{
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &details.capabilities);
    
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &formatCount, nullptr);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &presentModeCount, details.presentModes.data());
    }
    
    return details;
}

//----------------------------------------------------------------------------
/** Operations in GPUs require that commands are submitted to a queue.
 *  This helper function will find the Queue Family type that we need for draw commands **/
QueueFamilyIndices OttDevice::findQueueFamilies(VkPhysicalDevice physical_device) const
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &presentSupport);
        if (presentSupport)
            indices.presentFamily = i;
        
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphicsFamily = i;
        if (indices.isComplete())
            break;
        i++;
    }
    
    return indices;
}

//----------------------------------------------------------------------------
/** If none of the candidate formats support the desired usage,
 * then we can either return a special value or simply throw an exception. **/
VkFormat OttDevice::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
            return format;
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
            return format;
    }
    throw std::runtime_error("failed to find supported format!");
}

//----------------------------------------------------------------------------
/** Wrapper around the findSupportedFormat()
 * to query for the Depth specific format support on the GPU **/
VkFormat OttDevice::findDepthFormat()
{
    return findSupportedFormat( {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                                VK_IMAGE_TILING_OPTIMAL,
                                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

//----------------------------------------------------------------------------
/** Graphics cards can offer different types of memory to allocate from.
 *  Each type of memory varies in terms of allowed operations and performance characteristics. **/
uint32_t OttDevice::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags props)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & props) == props)
        {
            return i;
        }
    }
    throw std::runtime_error("failed to find suitable memory type!");
}

//----------------------------------------------------------------------------
/** Queries the device for specific parameters such as score and geometry shader availability. **/
int OttDevice::rateDeviceSuitability(VkPhysicalDevice physical_device)
{
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(physical_device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(physical_device, &deviceFeatures);

    int score = 0;

    // Discrete GPUs have a significant performance advantage
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        score += 1000;

    // Maximum possible size of textures affects graphics quality
    score += static_cast<int>(deviceProperties.limits.maxImageDimension2D);
    
    //Debug Log to detect the GPU.
    log_t<info>(DASHED_SEPARATOR);
    log_t<info>("GPU found.");
    log_t<info>("Name: {}", deviceProperties.deviceName);
    log_t<info>("Score: {}", score);
    log_t<info>("API Version: {}", deviceProperties.apiVersion);
    log_t<info>("Driver Version: {}", deviceProperties.driverVersion);
    log_t<info>(DASHED_SEPARATOR);
    
    return score;
}

//----------------------------------------------------------------------------
/** Stencil components are often used as intermediate objects/buffers to control which fragments should
 * proceed to the graphics pipelines and whose are not, similar to the depth buffer.
 * They have however a focus on masking and shadowing effects rather than depth perception.
 * This function polls the GPU to see if it supports Stencil Components. **/
bool OttDevice::hasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

//----------------------------------------------------------------------------
/** Basic check to see if the device is suitable to be used for our needs.
 *  It checks if the device supports Vulkan extensions, Swap Chain and
 *  features like sampler anisotropy. **/
bool OttDevice::isDeviceSuitable(VkPhysicalDevice physical_device,  std::vector<const char*> device_extensions )
{
    QueueFamilyIndices indices = findQueueFamilies(physical_device);
    const bool extensionsSupported = checkDeviceExtensionSupport(physical_device, device_extensions);

    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physical_device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }
    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(physical_device, &supportedFeatures);
    
    return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

//----------------------------------------------------------------------------
/** Called by isDeviceSuitable as an additional check
 *  to see if the Device has the Extensions Support needed. **/
bool OttDevice::checkDeviceExtensionSupport(VkPhysicalDevice physical_device, std::vector<const char*> device_extensions)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extensionCount, availableExtensions.data());
    std::set<std::string> requiredExtensions(device_extensions.begin(), device_extensions.end());

    for (const VkExtensionProperties extension : availableExtensions)
        requiredExtensions.erase(extension.extensionName);
    
    return requiredExtensions.empty();
}

//----------------------------------------------------------------------------
/** Called by isDeviceSuitable as an additional check
 *  to see if the Device has the Validation Layers support needed. **/
bool OttDevice::checkValidationLayerSupport(const std::vector<const char*> &validationLayers)
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers)
    {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }
        if (!layerFound)
            return false;
    }
    return true;
}
