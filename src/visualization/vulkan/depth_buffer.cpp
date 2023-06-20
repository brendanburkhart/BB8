#include "depth_buffer.hpp"

namespace visualization {
namespace vulkan {

DepthBuffer::DepthBuffer(const Device& device, uint32_t width, uint32_t height)
    : image(device, width, height, parametersFor(device)) {}

vk::Format DepthBuffer::getFormat() const {
    return image.getFormat();
}

vk::ImageView DepthBuffer::getView() const {
    return image.getView();
}

bool DepthBuffer::formatHasStencil(vk::Format format) {
    return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}

vk::Format DepthBuffer::findFormat(const Device& device) {
    std::vector<vk::Format> formats = {
        vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint};

    vk::FormatFeatureFlags depth_usage = vk::FormatFeatureFlagBits::eDepthStencilAttachment;
    for (auto format : formats) {
        if (device.supportsFormatUsage(format, tiling, depth_usage)) {
            return format;
        }
    }

    throw std::runtime_error("could not find a supported depth buffer format");
}

Image::Parameters DepthBuffer::parametersFor(const Device& device) {
    auto format = findFormat(device);
    auto memory = vk::MemoryPropertyFlagBits::eDeviceLocal;
    auto usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
    auto aspects = vk::ImageAspectFlagBits::eDepth;

    return Image::Parameters(memory, usage, tiling, format, aspects);
}

}  // namespace vulkan
}  // namespace visualization
