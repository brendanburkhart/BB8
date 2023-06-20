#ifndef BB8_VISUALIZATION_VULKAN_BUFFER_HPP
#define BB8_VISUALIZATION_VULKAN_BUFFER_HPP

#include <vulkan/vulkan_raii.hpp>

#include "device.hpp"

namespace visualization {
namespace vulkan {

class Buffer {
public:
    class Requirements {
    public:
        Requirements(size_t size,
                     vk::MemoryPropertyFlags properties,
                     vk::BufferUsageFlags usage,
                     vk::SharingMode sharing_mode,
                     bool keep_mapped);

        static Requirements staging(size_t size);
        static Requirements vertex(size_t size);
        static Requirements index(size_t size);
        static Requirements uniform(size_t size);

        size_t size;
        vk::MemoryPropertyFlags properties;
        vk::BufferUsageFlags usage;
        vk::SharingMode sharing_mode;
        bool keep_mapped;
    };

    Buffer(const Device& device, Requirements requirements);

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    Buffer(Buffer&&) = default;
    Buffer& operator=(Buffer&&) = default;

    ~Buffer();

    void fill(void* data, size_t size);

    size_t getSize() const;
    uint8_t* data() const;
    vk::Buffer get() const;

    vk::DescriptorBufferInfo descriptorInfo() const;

    static void copy(Buffer& source, Buffer& destination, const vk::CommandBuffer& command_buffer, const vk::Queue& transfer_queue);

private:
    static vk::raii::Buffer createBuffer(const Device& device, Requirements requirements);

    vk::raii::Buffer buffer;
    vk::raii::DeviceMemory memory;
    const size_t size;

    uint8_t* mapped_data = nullptr;
};

}  // namespace vulkan
}  // namespace visualization

#endif  // !define BB8_VISUALIZATION_VULKAN_BUFFER_HPP
