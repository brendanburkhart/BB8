#ifndef BB8_VISUALIZATION_BUFFER_HPP
#define BB8_VISUALIZATION_BUFFER_HPP

#include <vulkan/vulkan_raii.hpp>

namespace visualization {

class Buffer {
public:
    class Requirements {
    public:
        Requirements(size_t size,
                     vk::MemoryPropertyFlags properties,
                     vk::BufferUsageFlags usage,
                     vk::SharingMode sharing_mode, bool is_staging);

        static Requirements staging(size_t size);
        static Requirements vertex(size_t size);
        static Requirements index(size_t size);

        size_t size;
        vk::MemoryPropertyFlags properties;
        vk::BufferUsageFlags usage;
        vk::SharingMode sharing_mode;
        bool is_staging;
    };

    Buffer(const vk::PhysicalDevice& physical_sdevice, const vk::raii::Device& device, Requirements requirements);

    void fill(void* data, size_t size);

    vk::Buffer get();

    static void copy(Buffer& source, Buffer& destination, const vk::CommandBuffer& command_buffer, const vk::Queue& transfer_queue);

private:
    static uint32_t findMemoryType(const vk::PhysicalDevice& device, uint32_t required_type_bits, const vk::MemoryPropertyFlags& required_properties);
    static vk::raii::Buffer createBuffer(const vk::raii::Device& device, Requirements requirements);
    static vk::MemoryAllocateInfo memoryAllocationInfo(const vk::PhysicalDevice& device, const vk::MemoryRequirements& memory_reqs, Requirements buffer_reqs);

    vk::raii::Buffer buffer;
    vk::raii::DeviceMemory memory;
    const size_t size;
};

}  // namespace visualization

#endif  // !define BB8_VISUALIZATION_BUFFER_HPP
