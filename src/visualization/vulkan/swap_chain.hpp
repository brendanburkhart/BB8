#ifndef BB8_VISUALIZATION_VULKAN_SWAP_CHAIN_HPP
#define BB8_VISUALIZATION_VULKAN_SWAP_CHAIN_HPP

#include <limits>
#include <tuple>
#include <vector>
#include <vulkan/vulkan_raii.hpp>

#include "device.hpp"
#include "depth_buffer.hpp"

namespace visualization {
namespace vulkan {

class SwapChain {
public:
    class Support {
    public:
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> present_modes;

        static Support query(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface);
    };

    SwapChain(const Device& device,
              const vk::SurfaceKHR& surface,
              const vk::Extent2D window_size);

    void initializeFramebuffers(const Device& device, const vk::RenderPass& render_pass, const DepthBuffer& depth_buffer);

    std::tuple<vk::Result, uint32_t> acquireNextImage(const vk::Semaphore& semaphore);

    vk::Extent2D getExtent() const;
    vk::Format getFormat() const;

    size_t length() const;

    const vk::Framebuffer& getFramebuffer(size_t index) const;
    const vk::SwapchainKHR& get() const;

private:
    static vk::SurfaceFormatKHR chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& available_formats);
    static vk::Extent2D chooseExtent(const vk::SurfaceCapabilitiesKHR& capabilities, const vk::Extent2D window_size);
    static uint32_t chooseImageCount(const vk::SurfaceCapabilitiesKHR& capabilities);

    static constexpr uint64_t timeout = std::numeric_limits<uint64_t>::max();

    vk::Extent2D extent;
    vk::Format format;

    vk::raii::SwapchainKHR swap_chain;

    std::vector<vk::Image> images;
    std::vector<vk::raii::ImageView> image_views;
    std::vector<vk::raii::Framebuffer> framebuffers;
};

}  // namespace vulkan
}  // namespace visualization

#endif  // !BB8_VISUALIZATION_VULKAN_SWAP_CHAIN_HPP