#include "buffer.hpp"

namespace visualization {

Buffer::Requirements::Requirements(size_t size, vk::MemoryPropertyFlags properties, vk::BufferUsageFlags usage, vk::SharingMode sharing_mode, bool is_staging)
    : size(size), properties(properties), usage(usage), sharing_mode(sharing_mode), is_staging(is_staging) {}

Buffer::Requirements Buffer::Requirements::staging(size_t size) {
    auto properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
    auto usage = vk::BufferUsageFlagBits::eTransferSrc;
    return Requirements(size, properties, usage, vk::SharingMode::eExclusive, true);
}

Buffer::Requirements Buffer::Requirements::vertex(size_t size) {
    auto properties = vk::MemoryPropertyFlagBits::eDeviceLocal;
    auto usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;
    return Requirements(size, properties, usage, vk::SharingMode::eExclusive, true);
}

Buffer::Requirements Buffer::Requirements::index(size_t size) {
    auto properties = vk::MemoryPropertyFlagBits::eDeviceLocal;
    auto usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
    return Requirements(size, properties, usage, vk::SharingMode::eExclusive, true);
}

visualization::Buffer::Buffer(const vk::PhysicalDevice& physical_device, const vk::raii::Device& device, Requirements requirements)
    : buffer(createBuffer(device, requirements)),
      memory(vk::raii::DeviceMemory(device, memoryAllocationInfo(physical_device, buffer.getMemoryRequirements(), requirements))),
      size(requirements.size) {
    buffer.bindMemory(*memory, 0);
}

void Buffer::fill(void* data, size_t size) {
    uint8_t* mapped_data = static_cast<uint8_t*>(memory.mapMemory(0, size));
    std::memcpy(mapped_data, data, size);
    memory.unmapMemory();
}

vk::Buffer Buffer::get() {
    return *buffer;
}

void Buffer::copy(Buffer& source, Buffer& destination, const vk::CommandBuffer& command_buffer, const vk::Queue& transfer_queue) {
    if (source.size != destination.size) {
        throw std::runtime_error("souce and destination buffer sizes do not match");
    }

    auto begin_info = vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    command_buffer.begin(begin_info);

    auto copy_region = vk::BufferCopy(0, 0, source.size);
    command_buffer.copyBuffer(*source.buffer, *destination.buffer, copy_region);

    command_buffer.end();

    auto submit_info = vk::SubmitInfo({}, {}, command_buffer, {});
    transfer_queue.submit(submit_info);
    transfer_queue.waitIdle();
}

uint32_t Buffer::findMemoryType(const vk::PhysicalDevice& device, uint32_t required_type_bits, const vk::MemoryPropertyFlags& required_properties) {
    auto memory_properties = device.getMemoryProperties();
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

vk::raii::Buffer Buffer::createBuffer(const vk::raii::Device& device, Requirements requirements) {
    auto create_info = vk::BufferCreateInfo(vk::BufferCreateFlags(), requirements.size, requirements.usage, requirements.sharing_mode);
    return vk::raii::Buffer(device, create_info);
}

vk::MemoryAllocateInfo Buffer::memoryAllocationInfo(const vk::PhysicalDevice& device, const vk::MemoryRequirements& memory_reqs, Requirements buffer_reqs) {
    auto memory_type_index = findMemoryType(device, memory_reqs.memoryTypeBits, buffer_reqs.properties);
    return vk::MemoryAllocateInfo(memory_reqs.size, memory_type_index);
}

}  // namespace visualization