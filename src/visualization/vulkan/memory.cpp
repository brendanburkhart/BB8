#include "memory.hpp"

namespace visualization {
namespace vulkan {

uint32_t Memory::findType(const Device& device, uint32_t required_type_bits, const vk::MemoryPropertyFlags& required_properties) {
    auto memory_properties = device.physical().getMemoryProperties();
    for (uint32_t index = 0; index < memory_properties.memoryTypeCount; index++) {
        bool has_required_type = (1 << index) & required_type_bits;

        auto property_flags = memory_properties.memoryTypes[index].propertyFlags;
        bool has_required_properties = (property_flags & required_properties) == required_properties;
        if (has_required_type && has_required_properties) {
            return index;
        }
    }

    throw std::runtime_error("failed to find suitable memory type");
}

vk::MemoryAllocateInfo Memory::allocationInfo(const Device& device, const vk::MemoryRequirements& memory_reqs, const vk::MemoryPropertyFlags& required_properties) {
    auto memory_type_index = findType(device, memory_reqs.memoryTypeBits, required_properties);
    return vk::MemoryAllocateInfo(memory_reqs.size, memory_type_index);
}

}  // namespace vulkan
}  // namespace visualization
