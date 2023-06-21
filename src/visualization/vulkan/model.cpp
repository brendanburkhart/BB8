#include "model.hpp"

#include <string>
#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace visualization {
namespace vulkan {

Model Model::load(const Device& device, std::filesystem::path obj_file, std::filesystem::path texture_file) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    auto file_string = obj_file.string();
    const char* file = file_string.c_str();
    bool success = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, file);
    if (!success) {
        throw std::runtime_error(warn + err);
    }

    std::vector<shaders::Vertex> vertices;
    std::vector<uint32_t> indices;
    std::unordered_map<shaders::Vertex, uint32_t> vertex_indices{};

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            glm::vec3 position = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]};
            glm::vec2 texture_coordinate = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                // OBJ uses (0, 0) as lower left corner, need to invert
                1.0 - attrib.texcoords[2 * index.texcoord_index + 1]};
            glm::vec3 color = {1.0f, 1.0f, 1.0f};

            // if we already have seen vertex, reference previous copy
            auto vertex = shaders::Vertex(position, color, texture_coordinate);
            if (vertex_indices.count(vertex) == 0) {
                vertices.push_back(vertex);
                vertex_indices[vertex] = vertices.size() - 1;
            }

            indices.push_back(vertex_indices[vertex]);
        }
    }

    size_t vertices_size = vertices.size() * sizeof(vertices[0]);
    auto vertex_buffer = Buffer::load(device, vertices.data(), Buffer::Requirements::vertex(vertices_size));

    size_t indices_size = indices.size() * sizeof(indices[0]);
    auto index_buffer = Buffer::load(device, indices.data(), Buffer::Requirements::index(indices_size));

    auto texture = Texture::load(device, texture_file, vk::SamplerAddressMode::eRepeat);

    return Model(std::move(texture), std::move(vertex_buffer), std::move(index_buffer), indices.size());
}

uint32_t Model::indexCount() const {
    return index_count;
}

const Texture& Model::getTexture() const {
    return texture;
}

const Buffer& Model::getVertices() const {
    return vertex_buffer;
}

const Buffer& Model::getIndices() const {
    return index_buffer;
}

Model::Model(Texture texture, Buffer vertex_buffer, Buffer index_buffer, uint32_t index_count)
    : index_count(index_count), texture(std::move(texture)), vertex_buffer(std::move(vertex_buffer)), index_buffer(std::move(index_buffer)) {}

}  // namespace vulkan
}  // namespace visualization
