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
#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/hash.hpp>

#include <array>
#include <vector>
#include <volk.h>

namespace OttModel
{
    //----------------------------------------------------------------------------
    struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec2 texCoord;

        static VkVertexInputBindingDescription getBindingDescription()
        {
            constexpr VkVertexInputBindingDescription bindingDescription
            {
                .binding   = 0,
                .stride    = sizeof(Vertex),
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
            };
            return bindingDescription;
        }
    
        static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
        {
            std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
        
            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Vertex, pos);
        
            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex, color);

            attributeDescriptions[2].binding = 0;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[2].offset = offsetof(Vertex, texCoord);
        
            return attributeDescriptions;
        }
    
        bool operator==(const Vertex& other) const
        {
            return pos == other.pos && color == other.color && texCoord == other.texCoord;
        }
    };

    //----------------------------------------------------------------------------
    struct modelObject
    {
        uint32_t  startIndex;
        uint32_t  startVertex;
        uint32_t  indexCount;
        uint32_t  textureID;
        glm::vec3 pushColorID;
    };

    
} // namespace OttModel

//----------------------------------------------------------------------------
template<> struct std::hash<OttModel::Vertex>
{
    size_t operator()(OttModel::Vertex const& vertex) const
    {
        return ((std::hash<glm::vec3>()(vertex.pos) ^
               ( std::hash<glm::vec3>()(vertex.color)    << 1)) >> 1) ^
               ( std::hash<glm::vec2>()(vertex.texCoord) << 1);
    }
};