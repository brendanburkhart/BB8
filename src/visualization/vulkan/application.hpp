#ifndef BB8_VISUALIZATION_VULKAN_APPLICATION_HPP
#define BB8_VISUALIZATION_VULKAN_APPLICATION_HPP

#include <vulkan/vulkan_raii.hpp>

#include "buffer.hpp"
#include "device.hpp"
#include "frame_resources.hpp"
#include "shaders/vertex.hpp"
#include "swap_chain.hpp"
#include "texture.hpp"
#include "utilities.hpp"
#include "depth_buffer.hpp"
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

    static Texture createTexture(const Device& device, const vk::CommandPool& pool);

    Buffer buildVertexBuffer();
    Buffer buildIndexBuffer();

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
    vk::raii::CommandPool transient_pool;

    vk::raii::DescriptorPool descriptor_pool;

    Texture texture_image;
    DepthBuffer depth_buffer;

    const std::array<shaders::Vertex, 8> vertex_data = {
        // sqaure one
        shaders::Vertex({-0.5f, -0.5f, 0.0}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}),
        shaders::Vertex({0.5f, -0.5f, 0.0}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}),
        shaders::Vertex({0.5f, 0.5f, 0.0}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}),
        shaders::Vertex({-0.5f, 0.5f, 0.0}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}),
        // square two
        shaders::Vertex({-0.5f, -0.5f, -0.5}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}),
        shaders::Vertex({0.5f, -0.5f, -0.5}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}),
        shaders::Vertex({0.5f, 0.5f, -0.5}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}),
        shaders::Vertex({-0.5f, 0.5f, -0.5}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f})};
    const std::vector<uint16_t> vertex_indices = {
        0, 1, 2, 2, 3, 0,  // square one
        4, 5, 6, 6, 7, 4   // square two
    };

    Buffer vertex_buffer;
    Buffer index_buffer;

    static constexpr size_t max_frames_in_flight = 2;
    std::array<FrameResources, max_frames_in_flight> frames;
    size_t frame_index = 0;

    SwapChain swap_chain;
};

}  // namespace vulkan
}  // namespace visualization

#endif  // !BB8_VISUALIZATION_VULKAN_APPLICATION_HPP
