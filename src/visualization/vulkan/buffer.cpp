#include "buffer.hpp"

#include "memory.hpp"

namespace visualization {
namespace vulkan {

Buffer::Requirements::Requirements(size_t size, vk::MemoryPropertyFlags properties, vk::BufferUsageFlags usage, vk::SharingMode sharing_mode, bool keep_mapped)
    : size(size), properties(properties), usage(usage), sharing_mode(sharing_mode), keep_mapped(keep_mapped) {
    auto host_visible = vk::MemoryPropertyFlagBits::eHostVisible;
    bool can_map = (properties & host_visible) == host_visible;
    if (keep_mapped && !can_map) {
        throw std::runtime_error("Requirement::keep_mapped specified, but buffer will not be host visible");
    }
}

Buffer::Requirements Buffer::Requirements::staging(size_t size) {
    auto properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
    auto usage = vk::BufferUsageFlagBits::eTransferSrc;
    return Requirements(size, properties, usage, vk::SharingMode::eExclusive, false);
}

Buffer::Requirements Buffer::Requirements::vertex(size_t size) {
    auto properties = vk::MemoryPropertyFlagBits::eDeviceLocal;
    auto usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;
    return Requirements(size, properties, usage, vk::SharingMode::eExclusive, false);
}

Buffer::Requirements Buffer::Requirements::index(size_t size) {
    auto properties = vk::MemoryPropertyFlagBits::eDeviceLocal;
    auto usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
    return Requirements(size, properties, usage, vk::SharingMode::eExclusive, false);
}

Buffer::Requirements Buffer::Requirements::uniform(size_t size) {
    auto properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
    auto usage = vk::BufferUsageFlagBits::eUniformBuffer;
    return Requirements(size, properties, usage, vk::SharingMode::eExclusive, true);
}

Buffer::Buffer(const Device& device, Requirements requirements)
    : buffer(createBuffer(device, requirements)),
      memory(vk::raii::DeviceMemory(device.logical(), Memory::allocationInfo(device, buffer.getMemoryRequirements(), requirements.properties))),
      size(requirements.size) {
    buffer.bindMemory(*memory, 0);

    if (requirements.keep_mapped) {
        mapped_data = static_cast<uint8_t*>(memory.mapMemory(0, size));
    }
}

Buffer::~Buffer() {
    if (mapped_data != nullptr) {
        memory.unmapMemory();
        mapped_data = nullptr;
    }
}

void Buffer::fill(void* data, size_t size) {
    assert(size == this->size);
    mapped_data = static_cast<uint8_t*>(memory.mapMemory(0, size));
    std::memcpy(mapped_data, data, size);
    memory.unmapMemory();
    mapped_data = nullptr;
}

size_t Buffer::getSize() const {
    return size;
}

uint8_t* Buffer::data() const {
    assert(mapped_data != nullptr);
    return mapped_data;
}

vk::Buffer Buffer::get() const {
    return *buffer;
}

vk::DescriptorBufferInfo Buffer::descriptorInfo() const {
    return vk::DescriptorBufferInfo(*buffer, 0, size);
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

vk::raii::Buffer Buffer::createBuffer(const Device& device, Requirements requirements) {
    auto create_info = vk::BufferCreateInfo(vk::BufferCreateFlags(), requirements.size, requirements.usage, requirements.sharing_mode);
    return vk::raii::Buffer(device.logical(), create_info);
}

}  // namespace vulkan
}  // namespace visualization
