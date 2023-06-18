#ifndef BB8_VISUALIZATION_SHADERS_VERTEX_HPP
#define BB8_VISUALIZATION_SHADERS_VERTEX_HPP

#include <array>
#include <glm/glm.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace visualization {
namespace shaders {

class Vertex {
public:
    glm::vec2 position;
    glm::vec3 color;

    Vertex(glm::vec2 position, glm::vec3 color);

    static vk::VertexInputBindingDescription getBindingDescription();
    static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions();
};

}  // namespace shaders
}  // namespace visualization

#endif  // !BB8_VISUALIZATION_SHADERS_VERTEX_HPP
