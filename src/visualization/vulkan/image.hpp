#ifndef BB8_VISUALIZATION_VULKAN_IMAGE_HPP
#define BB8_VISUALIZATION_VULKAN_IMAGE_HPP

#include <filesystem>
#include <vulkan/vulkan_raii.hpp>

#include "buffer.hpp"
#include "device.hpp"

namespace visualization {
namespace vulkan {

class Image {
public:
    class Parameters {
    public:
        Parameters(vk::MemoryPropertyFlags memory_properties,
                   vk::ImageUsageFlags usage,
                   vk::ImageTiling tiling,
                   vk::Format format = vk::Format::eR8G8B8A8Srgb);

        static Parameters texture();

        vk::MemoryPropertyFlags memory_properties;
        vk::ImageUsageFlags usage;
        vk::ImageTiling tiling;
        vk::Format format;
    };

    static Image load(const Device& device,
                      std::filesystem::path image_file,
                      Parameters parameters,
                      const vk::CommandBuffer& command_buffer,
                      const vk::Queue& transfer_queue);

    vk::DescriptorImageInfo descriptorInfo() const;

private:
    Image(const Device& device, uint32_t width, uint32_t height, Parameters parameters);

    static vk::raii::Image createImage(const Device& device, vk::Extent3D extent, Parameters parameters);
    static vk::raii::DeviceMemory allocateMemory(const Device& device, vk::MemoryRequirements memory_requirements, vk::MemoryPropertyFlags property_requirements);
    static vk::raii::ImageView createView(const Device& device, const vk::Image& image, vk::Format format);
    static vk::raii::Sampler createSampler(const Device& device);

    void transitionLayout(const vk::CommandBuffer& command_buffer, vk::Format format, vk::ImageLayout new_layout);
    void fill(const vk::CommandBuffer& command_buffer, const Buffer& source);

    const vk::Extent3D extent;
    vk::ImageLayout layout;
    vk::Format format;

    vk::raii::Image image;
    vk::raii::DeviceMemory memory;
    vk::raii::ImageView view;
    vk::raii::Sampler sampler;
};

}  // namespace vulkan
}  // namespace visualization

#endif  // !BB8_VISUALIZATION_VULKAN_IMAGE_HPP
