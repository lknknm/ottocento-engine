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

#include "device.h"
#include "swapchain.h"
#include <volk.h>
#include <vector>

struct PushConstantData {
    alignas(16) glm::vec3 offset;
    alignas(16) glm::vec3 color;
    alignas(4)  uint32_t  textureID;
};

/** Wrapper that will act as a boilerplate to create multiple graphics pipelines.
 *  Must be instanced only once. VkPipelines are handled internally. **/
class OttPipeline
{
//----------------------------------------------------------------------------
public:
//----------------------------------------------------------------------------

    OttPipeline() = default;
    OttPipeline(OttDevice* device_reference, OttSwapChain* swapchain_reference);
    ~OttPipeline();

    OttPipeline(const OttPipeline&) = delete;
    void operator=(const OttPipeline&) = delete;

    VkPipelineLayout getPipelineLayout() const { return pipelineLayout; }
    
    VkPipelineVertexInputStateCreateInfo initVertexInputInfo (
        uint32_t vertex_binding_desc_count,
        VkVertexInputBindingDescription* binding_description,
        uint32_t att_desc_count,
        VkVertexInputAttributeDescription* vertex_att_desc
    );
    
    void createGraphicsPipeline (
        std::vector<VkDescriptorSetLayout>& descriptor_set_layouts,
        std::string vertex_shader_path, std::string fragment_shader_path,
        VkPipeline& pipeline, VkPipelineVertexInputStateCreateInfo vertex_input_info,
        VkPolygonMode polygon_mode
    );
    
    struct
    {
        VkPipeline grid;
        VkPipeline object;
        VkPipeline wireframe;
    } graphicsPipelines = nullptr;
    
//----------------------------------------------------------------------------
private:
//----------------------------------------------------------------------------
    VkPipelineLayout       pipelineLayout = nullptr;
    OttDevice*             pDevice;
    OttSwapChain*          pSwapchain;
    std::vector<VkDescriptorSetLayout>* pDescSetLayouts = nullptr;
    VkDevice               device;
    
    std::array<VkDynamicState, 2> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };
    
    void createPipelineLayout(std::vector<VkDescriptorSetLayout>& descriptor_set_layouts);

//----------------------------------------------------------------------------
// Struct initialization Helper functions ------------------------------------

    [[nodiscard]]           VkShaderModule                         createShaderModule         (const std::vector<char>& code);
    [[nodiscard]] constexpr VkPipelineShaderStageCreateInfo        initShaderStageCreateInfo  (VkShaderStageFlagBits stage, VkShaderModule module);
    [[nodiscard]] constexpr VkPipelineInputAssemblyStateCreateInfo initInputAssembly          ();
    [[nodiscard]] constexpr VkPipelineViewportStateCreateInfo      initViewportState          (uint32_t viewport_count,uint32_t scissor_count);
    [[nodiscard]] constexpr VkPipelineRasterizationStateCreateInfo initRasterizer             (VkPolygonMode polygon_mode, float line_width);
    [[nodiscard]] constexpr VkPipelineMultisampleStateCreateInfo   initMultisamplingState     (VkSampleCountFlagBits rasterization_samples);
    [[nodiscard]] constexpr VkPipelineDepthStencilStateCreateInfo  initDepthStencilInfo       ();
    [[nodiscard]] constexpr VkPipelineColorBlendAttachmentState    initColorBlendAttachment   ();
    [[nodiscard]] constexpr VkPipelineColorBlendStateCreateInfo    initColorBlendCreateInfo   (VkPipelineColorBlendAttachmentState* color_blend_attachment);
    [[nodiscard]] constexpr VkPipelineDynamicStateCreateInfo       initDynamicState           ();
    
// End of helper functions --------------------------------------------------
    
}; // class OttPipeline