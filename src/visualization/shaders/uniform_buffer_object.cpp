#include "uniform_buffer_object.hpp"

namespace visualization {
namespace shaders {

vk::DescriptorSetLayoutBinding visualization::shaders::UniformBufferObject::layoutBinding() {
    return vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex);
}

}  // namespace shaders
}  // namespace visualization
