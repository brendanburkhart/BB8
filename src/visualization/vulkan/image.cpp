#include "image.hpp"

#include <cmath>
#include <iostream>

#include "../resources/image.hpp"
#include "memory.hpp"

namespace visualization {
namespace vulkan {

Image::Parameters::Parameters(vk::MemoryPropertyFlags memory_properties,
                              vk::ImageUsageFlags usage,
                              vk::ImageTiling tiling,
                              vk::Format format,
                              vk::ImageAspectFlags aspects,
                              bool mipmap)
    : memory_properties(memory_properties),
      usage(usage),
      tiling(tiling),
      format(format),
      aspects(aspects),
      mipmap(mipmap) {
    // to generate a mipmap, we will need to transfer portions of the image to itself
    if (mipmap) {
        this->usage = usage | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst;
    }
}

Image Image::load(const Device& device,
                  std::filesystem::path image_file,
                  Parameters parameters) {
    // ensure we can transfer into the image
    parameters.usage = parameters.usage | vk::ImageUsageFlagBits::eTransferDst;

    resources::Image image_source = resources::Image::load(image_file);

    auto allocate_info = vk::CommandBufferAllocateInfo(device.transientPool(), vk::CommandBufferLevel::ePrimary, 1);
    auto command_buffers = vk::raii::CommandBuffers(device.logical(), allocate_info);
    auto command_buffer = std::move(command_buffers.front());

    auto begin_info = vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    command_buffer.begin(begin_info);

    Buffer staging = Buffer(device, Buffer::Requirements::staging(image_source.size()));
    staging.fill((void*)image_source.data(), image_source.size());

    auto image = Image(device, image_source.width(), image_source.height(), parameters);

    image.transitionLayout(*command_buffer, vk::ImageLayout::eTransferDstOptimal);
    image.fill(*command_buffer, staging);

    if (parameters.mipmap) {
        image.generateMIPMaps(*command_buffer, device);
    }

    image.transitionLayout(*command_buffer, vk::ImageLayout::eShaderReadOnlyOptimal);

    command_buffer.end();

    auto submit_info = vk::SubmitInfo({}, {}, *command_buffer, {});
    device.transferQueue().submit(submit_info);
    device.transferQueue().waitIdle();

    return image;
}

const vk::ImageView Image::getView() const {
    return *view;
}

vk::ImageLayout Image::getLayout() const {
    return layout;
}

vk::ImageTiling Image::getTiling() const {
    return tiling;
}

vk::Format Image::getFormat() const {
    return format;
}

uint32_t Image::getMIPMapLevels() const {
    return mip_levels;
}

Image::Image(const Device& device, uint32_t width, uint32_t height, Parameters parameters)
    : extent(width, height, 1),
      layout(vk::ImageLayout::eUndefined),
      tiling(parameters.tiling),
      format(parameters.format),
      aspects(parameters.aspects),
      mip_levels(parameters.mipmap ? computeMIPLevels(width, height) : 1),
      image(createImage(device, extent, mip_levels, parameters)),
      memory(allocateMemory(device, image.getMemoryRequirements(), parameters.memory_properties)),
      view(nullptr) {
    image.bindMemory(*memory, 0);
    // can't create image view until image memory is bound
    view = createView(device);
}

uint32_t Image::computeMIPLevels(uint32_t width, uint32_t height) {
    auto log = std::floor(std::log2(std::max(width, height)));
    return static_cast<uint32_t>(log + 1);
}

vk::raii::Image Image::createImage(const Device& device, vk::Extent3D extent, uint32_t mip_levels, Parameters parameters) {
    auto create_info = vk::ImageCreateInfo(
        {},                           // flags
        vk::ImageType::e2D,           // image type
        parameters.format,            // format
        extent,                       // extent
        mip_levels,                   // mip levels
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

vk::raii::ImageView Image::createView(const Device& device) {
    auto subresource = vk::ImageSubresourceRange(aspects, 0, mip_levels, 0, 1);
    auto view_info = vk::ImageViewCreateInfo({}, *image, vk::ImageViewType::e2D, format, {}, subresource);

    return vk::raii::ImageView(device.logical(), view_info);
}

void Image::generateMIPMaps(vk::CommandBuffer command_buffer, const Device& device) {
    if (!device.supportsFormatUsage(format, tiling, vk::FormatFeatureFlagBits::eSampledImageFilterLinear)) {
        throw std::runtime_error("image format does not support vk::Filter::Linear needed for generating mipmap");
    }

    auto subresource = vk::ImageSubresourceRange(aspects, 0, 1, 0, 1);
    auto barrier = vk::ImageMemoryBarrier(
        vk::AccessFlagBits::eTransferWrite,
        vk::AccessFlagBits::eTransferRead,
        layout,
        vk::ImageLayout::eTransferSrcOptimal,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        *image,
        subresource
    );

    int32_t mip_width = extent.width;
    int32_t mip_height = extent.height;
    for (uint32_t mip_level = 1; mip_level < mip_levels; mip_level++) {
        barrier.subresourceRange.baseMipLevel = mip_level - 1;

        auto transfer = vk::PipelineStageFlagBits::eTransfer;
        command_buffer.pipelineBarrier(transfer, transfer, {}, {}, {}, barrier);

        auto source = vk::ImageSubresourceLayers(aspects, mip_level - 1, 0, 1);
        auto destination = vk::ImageSubresourceLayers(aspects, mip_level, 0, 1);
        auto src_offsets = std::array<vk::Offset3D, 2>{vk::Offset3D(0, 0, 0), vk::Offset3D(mip_width, mip_height, 1)};
        mip_width = mip_width > 1 ? mip_width / 2 : 1;
        mip_height = mip_height > 1 ? mip_height / 2 : 1;
        auto dst_offsets = std::array<vk::Offset3D, 2>{vk::Offset3D(0, 0, 0), vk::Offset3D(mip_width, mip_height, 1)};
        auto blit = vk::ImageBlit(source, src_offsets, destination, dst_offsets);

        command_buffer.blitImage(*image, vk::ImageLayout::eTransferSrcOptimal, *image, vk::ImageLayout::eTransferDstOptimal, blit, vk::Filter::eLinear);
    }

    // Transition last layer
    barrier.subresourceRange.baseMipLevel = mip_levels - 1;
    auto transfer = vk::PipelineStageFlagBits::eTransfer;
    command_buffer.pipelineBarrier(transfer, transfer, {}, {}, {}, barrier);

    layout = vk::ImageLayout::eTransferSrcOptimal;
}

bool Image::formatHasStencil(vk::Format format) const {
    return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}

void Image::transitionLayout(const vk::CommandBuffer& command_buffer, vk::ImageLayout new_layout) {
    auto subresource = vk::ImageSubresourceRange(aspects, 0, mip_levels, 0, 1);
    if (formatHasStencil(format)) {
        subresource.aspectMask = subresource.aspectMask | vk::ImageAspectFlagBits::eStencil;
    }

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
    } else if (layout == vk::ImageLayout::eTransferSrcOptimal && new_layout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        source_stage = vk::PipelineStageFlagBits::eTransfer;
        destination_stage = vk::PipelineStageFlagBits::eFragmentShader;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
    } else {
        throw std::runtime_error("unsupport image layout transition");
    }

    command_buffer.pipelineBarrier(source_stage, destination_stage, {}, {}, {}, barrier);
    layout = new_layout;
}

void Image::fill(const vk::CommandBuffer& command_buffer, const Buffer& source) {
    assert(source.getSize() == 4 * extent.width * extent.height);
    auto subresource = vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1);
    auto region = vk::BufferImageCopy(0, 0, 0, subresource, {0, 0, 0}, extent);
    command_buffer.copyBufferToImage(source.get(), *image, layout, region);
}

}  // namespace vulkan
}  // namespace visualization
