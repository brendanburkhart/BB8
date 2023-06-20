#include "swap_chain.hpp"

#include <limits>
#include <stdexcept>

namespace visualization {
namespace vulkan {

SwapChain::Support SwapChain::Support::query(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface) {
    Support support;

    support.capabilities = device.getSurfaceCapabilitiesKHR(surface);
    support.formats = device.getSurfaceFormatsKHR(surface);
    support.present_modes = device.getSurfacePresentModesKHR(surface);

    return support;
}

SwapChain::SwapChain(
    const Device& device,
    const vk::SurfaceKHR& surface,
    const vk::Extent2D window_size)
    : swap_chain(nullptr) {
    auto support = Support::query(device.physical(), surface);
    auto surface_format = chooseSurfaceFormat(support.formats);
    extent = chooseExtent(support.capabilities, window_size);
    format = surface_format.format;

    constexpr uint32_t image_layers = 1;
    vk::SwapchainCreateInfoKHR create_into = vk::SwapchainCreateInfoKHR(
        vk::SwapchainCreateFlagsKHR(),
        surface,
        chooseImageCount(support.capabilities),
        surface_format.format,
        surface_format.colorSpace,
        extent,
        image_layers,
        vk::ImageUsageFlagBits::eColorAttachment,
        vk::SharingMode::eExclusive,
        {},
        support.capabilities.currentTransform,
        vk::CompositeAlphaFlagBitsKHR::eOpaque,
        vk::PresentModeKHR::eFifo,
        true,
        nullptr);

    swap_chain = vk::raii::SwapchainKHR(device.logical(), create_into);

    images = swap_chain.getImages();

    for (auto image : images) {
        auto image_view_create_info = vk::ImageViewCreateInfo({}, image, vk::ImageViewType::e2D, format, {}, {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
        image_views.emplace_back(device.logical(), image_view_create_info);
    }
}

void SwapChain::initializeFramebuffers(const Device& device, const vk::RenderPass& render_pass) {
    framebuffers.clear();

    for (auto& image_view : image_views) {
        auto framebuffer_create_info = vk::FramebufferCreateInfo({}, render_pass, *image_view, extent.width, extent.height, 1);
        framebuffers.emplace_back(device.logical(), framebuffer_create_info);
    }
}

std::tuple<vk::Result, uint32_t> SwapChain::acquireNextImage(const vk::Semaphore& semaphore) {
    return swap_chain.acquireNextImage(timeout, semaphore);
}

vk::Extent2D SwapChain::getExtent() const {
    return extent;
}

vk::Format SwapChain::getFormat() const {
    return format;
}

size_t SwapChain::length() const {
    return images.size();
}

const vk::Framebuffer& SwapChain::getFramebuffer(size_t index) const {
    return *framebuffers.at(index);
}

const vk::SwapchainKHR& SwapChain::get() const {
    return *swap_chain;
}

vk::SurfaceFormatKHR SwapChain::chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& available_formats) {
    if (available_formats.size() == 0) {
        throw std::runtime_error("no available surface formats");
    }

    for (const auto& surface_format : available_formats) {
        if (surface_format.format == vk::Format::eB8G8R8A8Srgb &&
            surface_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return surface_format;
        }
    }

    return available_formats[0];
}

vk::Extent2D SwapChain::chooseExtent(const vk::SurfaceCapabilitiesKHR& capabilities, const vk::Extent2D window_size) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        return vk::Extent2D(
            std::clamp(window_size.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
            std::clamp(window_size.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height));
    }
}

uint32_t SwapChain::chooseImageCount(const vk::SurfaceCapabilitiesKHR& capabilities) {
    uint32_t image_count = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0) {
        image_count = std::min(image_count, capabilities.maxImageCount);
    }

    return image_count;
}

}  // namespace vulkan
}  // namespace visualization
