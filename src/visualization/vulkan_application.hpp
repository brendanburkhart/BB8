#ifndef BB8_VISUALIZATION_VULKAN_APPLICATION_HPP
#define BB8_VISUALIZATION_VULKAN_APPLICATION_HPP

#include <optional>
#include <string>
#include <vector>
#include <vulkan/vulkan_raii.hpp>

#include "buffer.hpp"
#include "frame_resources.hpp"
#include "shaders/vertex.hpp"
#include "swap_chain.hpp"
#include "window.hpp"

namespace visualization {

class VulkanApplication {
public:
    VulkanApplication(std::string name, Window* window);

    void update();
    void exit();

    void onResize();

private:
    class QueueFamilyIndices {
    public:
        std::optional<uint32_t> graphics_family;
        std::optional<uint32_t> present_family;
    };

    static std::vector<const char*> gatherLayers(const std::vector<vk::LayerProperties> available_layers, const std::vector<std::string>& required_layers);
    static std::vector<const char*> gatherExtensions(const std::vector<vk::ExtensionProperties> available_extensions, const std::vector<std::string>& required_extensions);

    static QueueFamilyIndices findQueueFamilies(const vk::raii::PhysicalDevice& device, const vk::raii::SurfaceKHR& surface);

    static vk::raii::PhysicalDevice selectPhysicalDevice(const vk::raii::Instance& instance);

    static vk::raii::CommandPool createCommandPool(const vk::raii::Device& device, uint32_t queue_family_index);
    static vk::raii::CommandPool createTransientPool(const vk::raii::Device& device, uint32_t queue_family_index);

    static vk::raii::CommandBuffer createCommandBuffer(const vk::raii::Device& device, const vk::raii::CommandPool& command_pool);

    static vk::raii::DescriptorSetLayout buildDescriptorLayout(const vk::raii::Device& device);
    static vk::raii::Instance buildInstance(const vk::raii::Context& context, Window* window, std::string app_name, uint32_t app_version);
    static vk::raii::Device buildLogicalDevice(const QueueFamilyIndices& queue_family_indices, const vk::raii::PhysicalDevice& physical_device);
    static vk::raii::RenderPass buildRenderPass(const vk::raii::Device& device, vk::Format color_format);

    static vk::raii::DescriptorPool createDescriptorPool(const vk::raii::Device& device);

    Buffer buildVertexBuffer();
    Buffer buildIndexBuffer();

    void buildSwapChain();
    void buildGraphicsPipeline();

    void updateUniformBuffer();
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

    vk::raii::DescriptorSetLayout descriptor_set_layout;
    vk::raii::PipelineLayout pipeline_layout;
    vk::raii::RenderPass render_pass;
    vk::raii::Pipeline pipeline;

    vk::raii::CommandPool command_pool;
    vk::raii::CommandPool transient_pool;

    vk::raii::DescriptorPool descriptor_pool;

    const std::array<shaders::Vertex, 4> vertex_data = {
        shaders::Vertex({-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}),
        shaders::Vertex({0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}),
        shaders::Vertex({0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}),
        shaders::Vertex({-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f})};
    const std::vector<uint16_t> vertex_indices = {0, 1, 2, 2, 3, 0};
    Buffer vertex_buffer;
    Buffer index_buffer;

    static constexpr size_t max_frames_in_flight = 2;
    std::array<FrameResources, max_frames_in_flight> frames;
    size_t frame_index = 0;

    SwapChain swap_chain;
};

}  // namespace visualization

#endif  // !BB8_VISUALIZATION_VULKAN_APPLICATION_HPP
