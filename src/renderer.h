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

#include "device.h"
#include "swapchain.h"
#include "window.h"

class OttRenderer
{
//----------------------------------------------------------------------------
public:
//----------------------------------------------------------------------------
    
    OttRenderer(OttDevice* device_reference, OttSwapChain* swapchain_reference);
    ~OttRenderer();
    
    [[nodiscard]] uint32_t getCurrentFrameIndex() const { return currentFrameIndex; }
    [[nodiscard]] uint32_t getCurrentImageIndex() const { return currentImageIndex; }
    VkCommandBuffer getCurrentCommandBuffer() const {
        assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
        return commandBuffers[currentFrameIndex];
    }

    VkCommandBuffer beginFrame();
    void endFrame();
    void beginSwapChainRenderPass (VkCommandBuffer command_buffer);
    void endSwapChainRenderPass   (VkCommandBuffer command_buffer) const;
    
//----------------------------------------------------------------------------
private:
//----------------------------------------------------------------------------

    OttDevice* deviceRef;
    OttSwapChain* swapchainRef;
    
    std::vector<VkCommandBuffer> commandBuffers;
    
    uint32_t  currentFrameIndex  = 0;
    uint32_t  currentImageIndex  = 0;
    bool      framebufferResized = false;
    bool      isFrameStarted     = false;

    void createCommandBuffers();
};