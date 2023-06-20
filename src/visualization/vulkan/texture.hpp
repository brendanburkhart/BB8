#ifndef BB8_VISUALIZATION_VULKAN_TEXTURE_HPP
#define BB8_VISUALIZATION_VULKAN_TEXTURE_HPP

#include <vulkan/vulkan_raii.hpp>

#include "image.hpp"

namespace visualization {
namespace vulkan {

class Texture {
public:
    static Texture load(const Device& device,
                        std::filesystem::path image_file,
                        vk::SamplerAddressMode address_mode,
                        const vk::CommandBuffer& command_buffer,
                        const vk::Queue& transfer_queue);

    vk::DescriptorImageInfo descriptorInfo() const;

private:
    Texture(const Device& device, Image image, vk::SamplerAddressMode address_mode);

    static vk::raii::Sampler createSampler(const Device& device, vk::SamplerAddressMode address_mode);

    Image image;
    vk::raii::Sampler sampler;
};

}  // namespace vulkan
}  // namespace visualization

#endif  // !BB8_VISUALIZATION_VULKAN_TEXTURE_HPP
