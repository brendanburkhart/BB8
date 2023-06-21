#ifndef BB8_VISUALIZATION_VULKAN_APPLICATION_HPP
#define BB8_VISUALIZATION_VULKAN_APPLICATION_HPP

#include <vulkan/vulkan_raii.hpp>

#include "buffer.hpp"
#include "depth_buffer.hpp"
#include "device.hpp"
#include "frame_resources.hpp"
#include "model.hpp"
#include "shaders/vertex.hpp"
#include "swap_chain.hpp"
#include "texture.hpp"
#include "utilities.hpp"
#include "window.hpp"

namespace visualization {
namespace vulkan {

class Application {
public:
    Application(std::string name, Window* window);

    void update();
    void exit();

    void onResize();

private:
    class QueueFamilyIndices {
    public:
        std::optional<uint32_t> graphics_family;
        std::optional<uint32_t> present_family;
    };

    static vk::raii::DescriptorSetLayout buildDescriptorLayout(const Device& device);
    static vk::raii::Instance buildInstance(const vk::raii::Context& context, Window* window, std::string app_name, uint32_t app_version);
    static Device buildDevice(const vk::raii::Instance& instance, const vk::SurfaceKHR& surface);

    static vk::raii::DescriptorPool createDescriptorPool(const Device& device);

    Model createModel();

    void buildRenderPass();
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

    Device device;

    vk::raii::DescriptorSetLayout descriptor_set_layout;
    vk::raii::PipelineLayout pipeline_layout;
    vk::raii::RenderPass render_pass;
    vk::raii::Pipeline pipeline;

    vk::raii::CommandPool command_pool;

    vk::raii::DescriptorPool descriptor_pool;

    DepthBuffer depth_buffer;

    Model model;

    static constexpr size_t max_frames_in_flight = 2;
    std::array<FrameResources, max_frames_in_flight> frames;
    size_t frame_index = 0;

    SwapChain swap_chain;
};

}  // namespace vulkan
}  // namespace visualization

#endif  // !BB8_VISUALIZATION_VULKAN_APPLICATION_HPP
