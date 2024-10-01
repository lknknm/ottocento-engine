#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

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

        constexpr static auto getBindingDescription() -> VkVertexInputBindingDescription
        {
            return
            {
                .binding   = 0,
                .stride    = sizeof(Vertex),
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
            };
        }

		constexpr static auto getAttributeDescriptions()
				-> std::array<VkVertexInputAttributeDescription, 3> {
			return {
					VkVertexInputAttributeDescription{
													  .binding  = 0,
													  .location = 0,
													  .format   = VK_FORMAT_R32G32B32_SFLOAT,
													  .offset   = offsetof(Vertex,      pos)},

					VkVertexInputAttributeDescription{
													  .binding  = 0,
													  .location = 1,
													  .format   = VK_FORMAT_R32G32B32_SFLOAT,
													  .offset   = offsetof(Vertex,    color)},
					VkVertexInputAttributeDescription{
													  .binding  = 0,
													  .location = 2,
													  .format   = VK_FORMAT_R32G32_SFLOAT,
													  .offset   = offsetof(Vertex, texCoord)}
            };
		}

		bool operator==(const Vertex& other) const = default;
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