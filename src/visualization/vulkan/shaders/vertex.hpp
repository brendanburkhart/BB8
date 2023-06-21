#ifndef BB8_VISUALIZATION_VULKAN_SHADERS_VERTEX_HPP
#define BB8_VISUALIZATION_VULKAN_SHADERS_VERTEX_HPP

#include <array>
#include <vulkan/vulkan_raii.hpp>

#include "../glm.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace visualization {
namespace vulkan {
namespace shaders {

class Vertex {
public:
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 texture_coordinate;

    Vertex(glm::vec3 position, glm::vec3 color, glm::vec2 texture_coordinate);

    bool operator==(const Vertex& other) const;

    static vk::VertexInputBindingDescription getBindingDescription();
    static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions();
};

}  // namespace shaders
}  // namespace vulkan
}  // namespace visualization

template <>
struct std::hash<visualization::vulkan::shaders::Vertex> {
    size_t operator()(visualization::vulkan::shaders::Vertex const& vertex) const {
        auto a = std::hash<glm::vec3>()(vertex.position);
        auto b = std::hash<glm::vec3>()(vertex.color);
        auto c = std::hash<glm::vec2>()(vertex.texture_coordinate);

        return ((a ^ (b << 1)) >> 1) ^ (c << 1);
    }
};

#endif  // !BB8_VISUALIZATION_VULKAN_SHADERS_VERTEX_HPP
