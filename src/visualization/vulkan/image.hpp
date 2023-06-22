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
                   vk::Format format,
                   vk::ImageAspectFlags aspects,
                   bool mipmap);

        vk::MemoryPropertyFlags memory_properties;
        vk::ImageUsageFlags usage;
        vk::ImageTiling tiling;
        vk::Format format;
        vk::ImageAspectFlags aspects;
        bool mipmap;
    };

    static Image load(const Device& device,
                      std::filesystem::path image_file,
                      Parameters parameters);

    Image(const Device& device, uint32_t width, uint32_t height, Parameters parameters);

    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;

    Image(Image&&) = default;
    Image& operator=(Image&&) = default;

    ~Image() = default;

    const vk::ImageView getView() const;
    vk::ImageLayout getLayout() const;
    vk::ImageTiling getTiling() const;
    vk::Format getFormat() const;
    uint32_t getMIPMapLevels() const;

private:
    static uint32_t computeMIPLevels(uint32_t width, uint32_t height);
    static vk::raii::Image createImage(const Device& device, vk::Extent3D extent, uint32_t mip_levels, Parameters parameters);
    static vk::raii::DeviceMemory allocateMemory(const Device& device, vk::MemoryRequirements memory_requirements, vk::MemoryPropertyFlags property_requirements);

    vk::raii::ImageView createView(const Device& device);
    void generateMIPMaps(vk::CommandBuffer command_buffer, const Device& device);

    bool formatHasStencil(vk::Format format) const;

    void transitionLayout(const vk::CommandBuffer& command_buffer, vk::ImageLayout new_layout);
    void fill(const vk::CommandBuffer& command_buffer, const Buffer& source);

    vk::Extent3D extent;
    vk::ImageLayout layout;
    vk::ImageTiling tiling;
    vk::Format format;
    vk::ImageAspectFlags aspects;

    uint32_t mip_levels;

    vk::raii::Image image;
    vk::raii::DeviceMemory memory;
    vk::raii::ImageView view;
};

}  // namespace vulkan
}  // namespace visualization

#endif  // !BB8_VISUALIZATION_VULKAN_IMAGE_HPP
