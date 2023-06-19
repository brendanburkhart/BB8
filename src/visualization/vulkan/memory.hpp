#ifndef BB8_VISUALIZATION_VULKAN_MEMORY_HPP
#define BB8_VISUALIZATION_VULKAN_MEMORY_HPP

#include <vulkan/vulkan_raii.hpp>

#include "device.hpp"

namespace visualization {
namespace vulkan {

class Memory {
public:
    static uint32_t findType(const Device& device, uint32_t required_type_bits, const vk::MemoryPropertyFlags& required_properties);
    static vk::MemoryAllocateInfo allocationInfo(const Device& device, const vk::MemoryRequirements& memory_reqs, const vk::MemoryPropertyFlags& required_properties);
};

}  // namespace vulkan
}  // namespace visualization

#endif  // !BB8_VISUALIZATION_VULKAN_MEMORY_HPP
