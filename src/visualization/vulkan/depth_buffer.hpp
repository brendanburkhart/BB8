#ifndef BB8_VISUALIZATION_VULKAN_DEPTH_BUFFER_HPP
#define BB8_VISUALIZATION_VULKAN_DEPTH_BUFFER_HPP

#include "image.hpp"

namespace visualization {
namespace vulkan {

class DepthBuffer {
public:
    DepthBuffer(const Device& device, uint32_t width, uint32_t height);

    DepthBuffer(const DepthBuffer&) = delete;
    DepthBuffer& operator=(const DepthBuffer&) = delete;

    DepthBuffer(DepthBuffer&&) = default;
    DepthBuffer& operator=(DepthBuffer&&) = default;

    ~DepthBuffer() = default;

    vk::Format getFormat() const;
    vk::ImageView getView() const;

private:
    static bool formatHasStencil(vk::Format format);
    static vk::Format findFormat(const Device& device);
    static Image::Parameters parametersFor(const Device& device);

    static constexpr vk::ImageTiling tiling = vk::ImageTiling::eOptimal;

    Image image;
};

}  // namespace vulkan
}  // namespace visualization

#endif  // !BB8_VISUALIZATION_VULKAN_DEPTH_BUFFER_HPP
