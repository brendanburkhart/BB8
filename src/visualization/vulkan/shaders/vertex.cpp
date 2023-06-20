#include "vertex.hpp"

visualization::shaders::Vertex::Vertex(glm::vec2 position, glm::vec3 color, glm::vec2 texture_coordinate)
    : position(position), color(color), texture_coordinate(texture_coordinate) {}

vk::VertexInputBindingDescription visualization::shaders::Vertex::getBindingDescription() {
    return vk::VertexInputBindingDescription(0, sizeof(Vertex), vk::VertexInputRate::eVertex);
}

std::array<vk::VertexInputAttributeDescription, 3> visualization::shaders::Vertex::getAttributeDescriptions() {
    return {
        vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, position)),
        vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color)),
        vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, texture_coordinate))
    };
}
