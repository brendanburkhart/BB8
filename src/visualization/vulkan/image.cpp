#include "image.hpp"

#include "buffer.hpp"
#include "memory.hpp"
#include "resources/image.hpp"

namespace visualization {
namespace vulkan {

Image::Parameters::Parameters(vk::MemoryPropertyFlags memory_properties,
                              vk::ImageUsageFlags usage,
                              vk::ImageTiling tiling,
                              vk::Format format) : memory_properties(memory_properties), usage(usage), tiling(tiling), format(format) {}

Image::Parameters Image::Parameters::texture() {
    auto usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;

    return Parameters(vk::MemoryPropertyFlagBits::eDeviceLocal, usage, vk::ImageTiling::eOptimal);
}

Image Image::load(const Device& device, std::filesystem::path image_file, Parameters parameters) {
    resources::Image image_source = resources::Image::load(image_file);

    Buffer staging = Buffer(device, Buffer::Requirements::staging(image_source.size()));

    staging.fill((void*)image_source.data(), image_source.size());

    auto image_extent = vk::Extent3D(image_source.width(), image_source.height(), 1);
    auto create_info = vk::ImageCreateInfo(
        {},                           // flags
        vk::ImageType::e2D,           // image type
        parameters.format,            // format
        image_extent,                 // extent
        1,                            // mip levels
        1,                            // array layers
        vk::SampleCountFlagBits::e1,  // samples
        parameters.tiling,            // tiling
        parameters.usage,             // usage
        vk::SharingMode::eExclusive,  // sharing
        {},                           // queue family indices
        vk::ImageLayout::eUndefined   // initial layout
    );

    vk::raii::Image image(device.logical(), create_info);

    auto memory_requirements = image.getMemoryRequirements();
    auto memory_allocation = Memory::allocationInfo(device, memory_requirements, parameters.memory_properties);
    vk::raii::DeviceMemory memory(device.logical(), memory_allocation);

    image.bindMemory(*memory, 0);

    void Buffer::copy(Buffer & source, Buffer & destination, const vk::CommandBuffer& command_buffer, const vk::Queue& transfer_queue) {
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

    return Texture(std::move(image), std::move(memory));
}

Image::Texture(vk::raii::Image&& image, vk::raii::DeviceMemory&& memory) : image(std::move(image)), memory(std::move(memory)) {}

}  // namespace vulkan
}  // namespace visualization
