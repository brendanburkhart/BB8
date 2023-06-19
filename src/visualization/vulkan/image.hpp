#ifndef BB8_VISUALIZATION_VULKAN_IMAGE_HPP
#define BB8_VISUALIZATION_VULKAN_IMAGE_HPP

#include <filesystem>
#include <vulkan/vulkan_raii.hpp>

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

    static Image load(const Device& device, std::filesystem::path image_file, Parameters parameters);

private:
    Image(vk::raii::Image&& image, vk::raii::DeviceMemory&& memory);

    vk::raii::Image image;
    vk::raii::DeviceMemory memory;
};

}  // namespace vulkan
}  // namespace visualization

#endif  // !BB8_VISUALIZATION_VULKAN_IMAGE_HPP
