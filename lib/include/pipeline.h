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
    void createGraphicsPipeline(std::vector<VkDescriptorSetLayout>& descriptor_set_layouts);

    struct
    {
        VkPipeline grid;
        VkPipeline object;
    } graphicsPipelines;
    
//----------------------------------------------------------------------------
private:
//----------------------------------------------------------------------------
    VkPipelineLayout       pipelineLayout;
    OttDevice*             pDevice;
    OttSwapChain*          pSwapchain;
    std::vector<VkDescriptorSetLayout>* pDescSetLayouts;
    VkDevice               device;
    
    VkShaderModule createShaderModule(const std::vector<char>& code);
    void createPipelineLayout(std::vector<VkDescriptorSetLayout>& descriptor_set_layouts);
};