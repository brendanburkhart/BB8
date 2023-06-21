#ifndef BB8_VISUALIZATION_VULKAN_MODEL_HPP
#define BB8_VISUALIZATION_VULKAN_MODEL_HPP

#include <filesystem>
#include <vector>

#include "shaders/vertex.hpp"
#include "texture.hpp"

namespace visualization {
namespace vulkan {

class Model {
public:
    static Model load(const Device& device, std::filesystem::path obj_file, std::filesystem::path texture_file);

    uint32_t indexCount() const;
    const Texture& getTexture() const;
    const Buffer& getVertices() const;
    const Buffer& getIndices() const;

private:
    Model(Texture texture, Buffer vertex_buffer, Buffer index_buffer, uint32_t index_count);

    uint32_t index_count;
    Texture texture;
    Buffer vertex_buffer;
    Buffer index_buffer;
};

}  // namespace vulkan
}  // namespace visualization

#endif  // !BB8_VISUALIZATION_VULKAN_MODEL_HPP
