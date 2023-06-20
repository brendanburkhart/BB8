#ifndef BB8_VISUALIZATION_SHADERS_VERTEX_HPP
#define BB8_VISUALIZATION_SHADERS_VERTEX_HPP

#include <array>
#include "../glm.hpp"
#include <vulkan/vulkan_raii.hpp>

namespace visualization {
namespace shaders {

class Vertex {
public:
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 texture_coordinate;

    Vertex(glm::vec3 position, glm::vec3 color, glm::vec2 texture_coordinate);

    static vk::VertexInputBindingDescription getBindingDescription();
    static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions();
};

}  // namespace shaders
}  // namespace visualization

#endif  // !BB8_VISUALIZATION_SHADERS_VERTEX_HPP
