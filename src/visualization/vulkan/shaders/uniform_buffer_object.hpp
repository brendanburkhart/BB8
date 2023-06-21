#ifndef BB8_VISUALIZATION_VULKAN_SHADERS_UNIFORM_BUFFER_OBJECT_HPP
#define BB8_VISUALIZATION_VULKAN_SHADERS_UNIFORM_BUFFER_OBJECT_HPP

#include <vulkan/vulkan_raii.hpp>

#include "../glm.hpp"

namespace visualization {
namespace vulkan {
namespace shaders {

class UniformBufferObject {
public:
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 projection;

    static vk::DescriptorSetLayoutBinding layoutBinding();
};

}  // namespace shaders
}  // namespace vulkan
}  // namespace visualization

#endif  // !BB8_VISUALIZATION_VULKAN_SHADERS_UNIFORM_BUFFER_OBJECT_HPP
