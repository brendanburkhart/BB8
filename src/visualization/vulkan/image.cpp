#include "image.hpp"

#include "../resources/image.hpp"
#include "memory.hpp"

namespace visualization {
namespace vulkan {

Image::Parameters::Parameters(vk::MemoryPropertyFlags memory_properties,
                              vk::ImageUsageFlags usage,
                              vk::ImageTiling tiling,
                              vk::Format format,
                              vk::ImageAspectFlags aspects)
    : memory_properties(memory_properties), usage(usage), tiling(tiling), format(format), aspects(aspects) {}

Image::Parameters Image::Parameters::texture() {
    auto usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;

    return Parameters(vk::MemoryPropertyFlagBits::eDeviceLocal, usage, vk::ImageTiling::eOptimal);
}

Image Image::load(const Device& device,
                  std::filesystem::path image_file,
                  Parameters parameters,
                  const vk::CommandBuffer& command_buffer,
                  const vk::Queue& transfer_queue) {
    resources::Image image_source = resources::Image::load(image_file);

    Buffer staging = Buffer(device, Buffer::Requirements::staging(image_source.size()));
    staging.fill((void*)image_source.data(), image_source.size());

    auto image = Image(device, image_source.width(), image_source.height(), parameters);

    auto begin_info = vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    command_buffer.begin(begin_info);

    image.transitionLayout(command_buffer, parameters.format, vk::ImageLayout::eTransferDstOptimal);
    image.fill(command_buffer, staging);
    image.transitionLayout(command_buffer, parameters.format, vk::ImageLayout::eShaderReadOnlyOptimal);

    command_buffer.end();

    auto submit_info = vk::SubmitInfo({}, {}, command_buffer, {});
    transfer_queue.submit(submit_info);
    transfer_queue.waitIdle();

    return image;
}

const vk::ImageView Image::getView() const {
    return *view;
}

vk::ImageLayout Image::getLayout() const {
    return layout;
}

vk::Format Image::getFormat() const {
    return format;
}

Image::Image(const Device& device, uint32_t width, uint32_t height, Parameters parameters)
    : extent(width, height, 1),
      layout(vk::ImageLayout::eUndefined),
      format(parameters.format),
      aspects(parameters.aspects),
      image(createImage(device, extent, parameters)),
      memory(allocateMemory(device, image.getMemoryRequirements(), parameters.memory_properties)),
      view(nullptr) {
    image.bindMemory(*memory, 0);
    // can't create image view until image memory is bound
    view = createView(device, *image, format, aspects);
}

vk::raii::Image Image::createImage(const Device& device, vk::Extent3D extent, Parameters parameters) {
    auto create_info = vk::ImageCreateInfo(
        {},                           // flags
        vk::ImageType::e2D,           // image type
        parameters.format,            // format
        extent,                       // extent
        1,                            // mip levels
        1,                            // array layers
        vk::SampleCountFlagBits::e1,  // samples
        parameters.tiling,            // tiling
        parameters.usage,             // usage
        vk::SharingMode::eExclusive,  // sharing
        {},                           // queue family indices
        vk::ImageLayout::eUndefined   // initial layout
    );

    return vk::raii::Image(device.logical(), create_info);
}

vk::raii::DeviceMemory Image::allocateMemory(const Device& device, vk::MemoryRequirements memory_requirements, vk::MemoryPropertyFlags property_requirements) {
    auto memory_allocation = Memory::allocationInfo(device, memory_requirements, property_requirements);
    return vk::raii::DeviceMemory(device.logical(), memory_allocation);
}

vk::raii::ImageView Image::createView(const Device& device, const vk::Image& image, vk::Format format, vk::ImageAspectFlags aspects) {
    auto subresource = vk::ImageSubresourceRange(aspects, 0, 1, 0, 1);
    auto view_info = vk::ImageViewCreateInfo({}, image, vk::ImageViewType::e2D, format, {}, subresource);

    return vk::raii::ImageView(device.logical(), view_info);
}

void Image::transitionLayout(const vk::CommandBuffer& command_buffer, vk::Format format, vk::ImageLayout new_layout) {
    auto subresource = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
    auto barrier = vk::ImageMemoryBarrier({}, {}, layout, new_layout, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, *image, subresource);

    vk::PipelineStageFlags source_stage;
    vk::PipelineStageFlags destination_stage;

    if (layout == vk::ImageLayout::eUndefined && new_layout == vk::ImageLayout::eTransferDstOptimal) {
        source_stage = vk::PipelineStageFlagBits::eTopOfPipe;
        destination_stage = vk::PipelineStageFlagBits::eTransfer;
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
    } else if (layout == vk::ImageLayout::eTransferDstOptimal && new_layout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        source_stage = vk::PipelineStageFlagBits::eTransfer;
        destination_stage = vk::PipelineStageFlagBits::eFragmentShader;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
    } else {
        throw std::runtime_error("unsupport image layout transition");
    }

    command_buffer.pipelineBarrier(source_stage, destination_stage, {}, {}, {}, barrier);
    layout = new_layout;
    this->format = format;  // TODO: why do we need format
}

void Image::fill(const vk::CommandBuffer& command_buffer, const Buffer& source) {
    assert(source.getSize() == 4 * extent.width * extent.height);
    auto subresource = vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1);
    auto region = vk::BufferImageCopy(0, 0, 0, subresource, {0, 0, 0}, extent);
    command_buffer.copyBufferToImage(source.get(), *image, layout, region);
}

}  // namespace vulkan
}  // namespace visualization
