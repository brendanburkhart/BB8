#ifndef BB8_VISUALIZATION_VULKAN_APPLICATION_HPP
#define BB8_VISUALIZATION_VULKAN_APPLICATION_HPP

#include <optional>
#include <string>
#include <vector>
#include <vulkan/vulkan_raii.hpp>

#include "frame_sync.hpp"
#include "swap_chain.hpp"
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
    };

    static std::vector<const char*> gatherLayers(const std::vector<vk::LayerProperties> available_layers, const std::vector<std::string>& required_layers);
    static std::vector<const char*> gatherExtensions(const std::vector<vk::ExtensionProperties> available_extensions, const std::vector<std::string>& required_extensions);

    static QueueFamilyIndices findQueueFamilies(const vk::raii::PhysicalDevice& device, const vk::raii::SurfaceKHR& surface);

    static vk::raii::PhysicalDevice selectPhysicalDevice(const vk::raii::Instance& instance);

    static vk::raii::CommandPool createCommandPool(const vk::raii::Device& device, uint32_t queue_family_index);
    static vk::raii::CommandBuffer createCommandBuffer(const vk::raii::Device& device, const vk::raii::CommandPool& command_pool);

    static vk::raii::Instance buildInstance(const vk::raii::Context& context, Window* window, std::string app_name, uint32_t app_version);
    static vk::raii::Device buildLogicalDevice(const QueueFamilyIndices& queue_family_indices, const vk::raii::PhysicalDevice& physical_device);

    void buildRenderPass();
    void buildGraphicsPipeline();

    void recordCommandBuffer(vk::CommandBuffer command_buffer, const vk::Framebuffer& framebuffer);

    void drawFrame();

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

    SwapChain swap_chain;

    vk::raii::PipelineLayout pipeline_layout;
    vk::raii::RenderPass render_pass;
    vk::raii::Pipeline pipeline;

    vk::raii::CommandPool command_pool;

    static constexpr size_t max_frames_in_flight = 2;
    std::array<FrameSync, max_frames_in_flight> frame_sync;
    size_t frame_index = 0;
};

}  // namespace visualization

#endif  // !BB8_VISUALIZATION_VULKAN_APPLICATION_HPP
