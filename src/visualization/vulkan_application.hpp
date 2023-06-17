#ifndef VULKAN_APPLICATION_HPP
#define VULKAN_APPLICATION_HPP

#include <optional>
#include <string>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "window.hpp"

namespace visualization {

class VulkanApplication {
public:
    VulkanApplication(std::string name, Window* window);

    void run();

private:
    class QueueFamilyIndices {
    public:
        std::optional<uint32_t> graphics_family;
        std::optional<uint32_t> present_family;
    };

    class SwapChainSupportDetails {
    public:
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> present_modes;
    };

    static std::vector<const char*> gatherLayers(const std::vector<vk::LayerProperties> available_layers, const std::vector<std::string>& required_layers);
    static std::vector<const char*> gatherExtensions(const std::vector<vk::ExtensionProperties> available_extensions, const std::vector<std::string>& required_extensions);

    static QueueFamilyIndices findQueueFamilies(const vk::raii::PhysicalDevice& device, const vk::raii::SurfaceKHR& surface);

    static SwapChainSupportDetails querySwapChainSupport(const vk::raii::PhysicalDevice& device, const vk::raii::SurfaceKHR& surface);
    static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& available_formats);
    static vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, const Window* window);

    static vk::raii::PhysicalDevice selectPhysicalDevice(const vk::raii::Instance& instance);

    static vk::raii::CommandPool createCommandPool(const vk::raii::Device& device, uint32_t queue_family_index);
    static vk::raii::CommandBuffer createCommandBuffer(const vk::raii::Device& device, const vk::raii::CommandPool& command_pool);

    static vk::raii::Instance buildInstance(const vk::raii::Context& context, Window* window, std::string app_name, uint32_t app_version);
    static vk::raii::Device buildLogicalDevice(const QueueFamilyIndices& queue_family_indices, const vk::raii::PhysicalDevice& physical_device);

    void buildSwapChain(SwapChainSupportDetails support);
    void buildRenderPass();
    void buildGraphicsPipeline();

    void recordCommandBuffer(vk::CommandBuffer command_buffer, uint32_t image_index);

    void drawFrame();

    // bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    // SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    // bool isDeviceSuitable(VkPhysicalDevice device);
    // void selectPhysicalDevice();
    // void createLogicalDevice();
    // void initializeVulkan();

    static const std::vector<std::string> validation_layers;
    static const std::vector<std::string> device_extensions;

    static constexpr uint32_t api_version = VK_API_VERSION_1_1;

#ifdef NDEBUG
    static constexpr bool enable_validation_layers = false;
#else
    static constexpr bool enable_validation_layers = true;
#endif

    Window* window;

    vk::raii::Context context;
    vk::raii::Instance instance;

    vk::raii::SurfaceKHR surface;

    vk::raii::PhysicalDevice physical_device;

    QueueFamilyIndices queue_family_indices;
    vk::raii::Device device;
    vk::raii::Queue graphics_queue;
    vk::raii::Queue present_queue;

    vk::raii::SwapchainKHR swap_chain;
    vk::Extent2D swap_chain_extent;
    vk::Format swap_chain_format;
    std::vector<vk::Image> swap_chain_images;
    std::vector<vk::raii::Framebuffer> swap_chain_framebuffers;
    std::vector<vk::raii::ImageView> swap_chain_image_views;

    vk::raii::PipelineLayout pipeline_layout;
    vk::raii::RenderPass render_pass;
    vk::raii::Pipeline pipeline;

    vk::raii::CommandPool command_pool;
    vk::raii::CommandBuffer command_buffer;

    vk::raii::Semaphore image_available_semaphore;
    vk::raii::Semaphore render_finished_semaphore;
    vk::raii::Fence frame_in_flight_fence;
};

}  // namespace visualization

#endif  // !VULKAN_APPLICATION_HPP
