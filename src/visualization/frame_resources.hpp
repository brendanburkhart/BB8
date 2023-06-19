#ifndef BB8_VISUALIZATION_FRAME_RESOURCES_HPP
#define BB8_VISUALIZATION_FRAME_RESOURCES_HPP

#include <limits>
#include <vulkan/vulkan_raii.hpp>

#include "buffer.hpp"
#include "shaders/uniform_buffer_object.hpp"
#include "swap_chain.hpp"

namespace visualization {

class FrameResources {
public:
    FrameResources(const vk::PhysicalDevice& physical_device, const vk::raii::Device& device,
                   const vk::CommandPool& command_pool,
                   const vk::DescriptorPool& descriptor_pool, const vk::DescriptorSetLayout& descriptor_layout);

    const vk::CommandBuffer& getCommandBuffer() const;

    void waitUntilReady(const vk::Device& device) const;
    void reset(const vk::Device& device);

    std::tuple<vk::Result, uint32_t> acquireNextImage(SwapChain& swap_chain);

    void writeUniformBuffer(const shaders::UniformBufferObject& ubo);
    const vk::DescriptorSet& getDescriptor() const;

    void submitTo(vk::raii::Queue& graphics_queue);
    vk::Result presentTo(vk::raii::Queue& present_queue, SwapChain& swap_chain, uint32_t image_index);

private:
    static vk::raii::CommandBuffer createCommandBuffer(const vk::raii::Device& device, const vk::CommandPool& command_pool);
    static vk::raii::DescriptorSet createUBODescriptor(const vk::raii::Device& device, const vk::DescriptorPool& descriptor_pool,
        const vk::DescriptorSetLayout& descriptor_layout, Buffer& ubo_buffer);

    static constexpr uint64_t timeout = std::numeric_limits<uint64_t>::max();

    vk::raii::CommandBuffer command_buffer;

    Buffer ubo_buffer;
    vk::raii::DescriptorSet ubo_descriptor;

    vk::raii::Semaphore image_available_semaphore;
    vk::raii::Semaphore render_finished_semaphore;

    vk::raii::Fence in_flight_fence;
};

}  // namespace visualization

#endif  // !BB8_VISUALIZATION_FRAME_RESOURCES_HPP