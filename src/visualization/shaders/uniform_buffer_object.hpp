#ifndef BB8_VISUALIZATION_SHADERS_UNIFORM_BUFFER_OBJECT_HPP
#define BB8_VISUALIZATION_SHADERS_UNIFORM_BUFFER_OBJECT_HPP

#include <glm/glm.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace visualization {
namespace shaders {

class UniformBufferObject {
public:
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;

    static vk::DescriptorSetLayoutBinding layoutBinding();
};

}  // namespace shaders
}  // namespace visualization

#endif  // !BB8_VISUALIZATION_SHADERS_UNIFORM_BUFFER_OBJECT_HPP
