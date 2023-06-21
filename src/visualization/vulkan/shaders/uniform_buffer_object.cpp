#include "uniform_buffer_object.hpp"

namespace visualization {
namespace vulkan {
namespace shaders {

vk::DescriptorSetLayoutBinding UniformBufferObject::layoutBinding() {
    return vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex);
}

}  // namespace shaders
}  // namespace vulkan
}  // namespace visualization
