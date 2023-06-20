#ifndef BB8_VISUALIZATION_VULKAN_FRAME_RESOURCES_HPP
#define BB8_VISUALIZATION_VULKAN_FRAME_RESOURCES_HPP

#include <limits>
#include <vulkan/vulkan_raii.hpp>

#include "buffer.hpp"
#include "device.hpp"
#include "image.hpp"
#include "shaders/uniform_buffer_object.hpp"
#include "swap_chain.hpp"

namespace visualization {
namespace vulkan {

class FrameResources {
public:
    FrameResources(const Device& device,
                   const vk::CommandPool& command_pool,
                   const vk::DescriptorPool& descriptor_pool,
                   const vk::DescriptorSetLayout& layout,
                   const Image& texture);

    const vk::CommandBuffer& getCommandBuffer() const;

    void waitUntilReady(const Device& device) const;
    void reset(const Device& device);

    std::tuple<vk::Result, uint32_t> acquireNextImage(SwapChain& swap_chain);

    void writeUniformBuffer(const shaders::UniformBufferObject& ubo);
    const vk::DescriptorSet& getDescriptors() const;

    void submitTo(const vk::Queue& graphics_queue);
    vk::Result presentTo(const vk::Queue& present_queue, SwapChain& swap_chain, uint32_t image_index);

private:
    static vk::raii::CommandBuffer createCommandBuffer(const Device& device, const vk::CommandPool& command_pool);
    static vk::raii::DescriptorSet createDescriptors(const Device& device,
                                                     const vk::DescriptorPool& pool,
                                                     const vk::DescriptorSetLayout& layout,
                                                     const Buffer& ubo_buffer,
                                                     const Image& texture);

    static constexpr uint64_t timeout = std::numeric_limits<uint64_t>::max();

    vk::raii::CommandBuffer command_buffer;

    Buffer ubo_buffer;

    vk::raii::DescriptorSet descriptor_set;

    vk::raii::Semaphore image_available_semaphore;
    vk::raii::Semaphore render_finished_semaphore;

    vk::raii::Fence in_flight_fence;
};

}  // namespace vulkan
}  // namespace visualization

#endif  // !BB8_VISUALIZATION_VULKAN_FRAME_RESOURCES_HPP