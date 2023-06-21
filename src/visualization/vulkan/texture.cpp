#include "texture.hpp"

namespace visualization {
namespace vulkan {

Texture Texture::load(const Device& device, std::filesystem::path texture_file, vk::SamplerAddressMode address_mode) {
    auto usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
    auto parameters = Image::Parameters(vk::MemoryPropertyFlagBits::eDeviceLocal, usage, vk::ImageTiling::eOptimal);

    return Texture(device, Image::load(device, texture_file, parameters), address_mode);
}

vk::DescriptorImageInfo Texture::descriptorInfo() const {
    return vk::DescriptorImageInfo(*sampler, image.getView(), image.getLayout());
}

Texture::Texture(const Device& device, Image image, vk::SamplerAddressMode address_mode)
    : image(std::move(image)), sampler(createSampler(device, address_mode)){};

vk::raii::Sampler Texture::createSampler(const Device& device, vk::SamplerAddressMode address_mode) {
    bool enable_anisotropy = device.features().samplerAnisotropy;
    auto max_anisotropy = device.properties().limits.maxSamplerAnisotropy;

    auto sampler_info = vk::SamplerCreateInfo(
        {},                                // flags
        vk::Filter::eLinear,               // mag(nification) filter
        vk::Filter::eLinear,               // min(imization) filter
        vk::SamplerMipmapMode::eLinear,    // mipmap mode
        address_mode,                      // U address mode
        address_mode,                      // V address mode
        address_mode,                      // W address mode
        0.0,                               // mipmap LOD (level-of-detail) bias
        enable_anisotropy,                 // enable anisotropy
        max_anisotropy,                    // max anisotropy
        false,                             // enable compare
        vk::CompareOp::eAlways,            // compare op
        0.0,                               // min LOD
        0.0,                               // max LOD
        vk::BorderColor::eIntOpaqueBlack,  // border color for clamp-to-border address mode
        false                              // unnormalized coordinates
    );

    return vk::raii::Sampler(device.logical(), sampler_info);
}

}  // namespace vulkan
}  // namespace visualization
