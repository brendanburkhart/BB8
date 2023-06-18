#ifndef BB8_VISUALIZATION_FRAME_SYNC_HPP
#define BB8_VISUALIZATION_FRAME_SYNC_HPP

#include <limits>
#include <vulkan/vulkan_raii.hpp>

#include "swap_chain.hpp"

namespace visualization {

class FrameSync {
public:
    FrameSync(const vk::raii::Device& device, const vk::CommandPool& command_pool);

    const vk::CommandBuffer& getCommandBuffer() const;

    void waitUntilReady(const vk::Device& device) const;
    void reset(const vk::Device& device);

    std::tuple<vk::Result, uint32_t> acquireNextImage(SwapChain& swap_chain);

    void submitTo(vk::raii::Queue& graphics_queue);
    vk::Result presentTo(vk::raii::Queue& present_queue, SwapChain& swap_chain, uint32_t image_index);

private:
    static vk::raii::CommandBuffer createCommandBuffer(const vk::raii::Device& device, const vk::CommandPool& command_pool);

    static constexpr uint64_t timeout = std::numeric_limits<uint64_t>::max();

    vk::raii::CommandBuffer command_buffer;

    vk::raii::Semaphore image_available_semaphore;
    vk::raii::Semaphore render_finished_semaphore;

    vk::raii::Fence in_flight_fence;
};

}  // namespace visualization

#endif  // !BB8_VISUALIZATION_FRAME_SYNC_HPP