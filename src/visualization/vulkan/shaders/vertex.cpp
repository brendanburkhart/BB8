#include "vertex.hpp"

namespace visualization {
namespace vulkan {
namespace shaders {

Vertex::Vertex(glm::vec3 position, glm::vec3 color, glm::vec2 texture_coordinate)
    : position(position), color(color), texture_coordinate(texture_coordinate) {}

bool Vertex::operator==(const Vertex& other) const {
    return position == other.position && color == other.color && texture_coordinate == other.texture_coordinate;
}

vk::VertexInputBindingDescription Vertex::getBindingDescription() {
    return vk::VertexInputBindingDescription(0, sizeof(Vertex), vk::VertexInputRate::eVertex);
}

std::array<vk::VertexInputAttributeDescription, 3> Vertex::getAttributeDescriptions() {
    return {
        vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, position)),
        vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color)),
        vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, texture_coordinate))};
}

}  // namespace shaders
}  // namespace vulkan
}  // namespace visualization
