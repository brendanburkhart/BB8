#include "frame_sync.hpp"

namespace visualization {

FrameSync::FrameSync(const vk::raii::Device& device, const vk::CommandPool& command_pool)
    : command_buffer(createCommandBuffer(device, command_pool)),
      image_available_semaphore(device, vk::SemaphoreCreateInfo()),
      render_finished_semaphore(device, vk::SemaphoreCreateInfo()),
      in_flight_fence(device, vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled)) {}

const vk::CommandBuffer& FrameSync::getCommandBuffer() const {
    return *command_buffer;
}

void FrameSync::waitUntilReady(const vk::Device& device) const {
    vk::Result wait_result = device.waitForFences(*in_flight_fence, true, timeout);
    assert(wait_result == vk::Result::eSuccess);
    device.resetFences(*in_flight_fence);
}

std::tuple<vk::Result, uint32_t> FrameSync::acquireNextImage(SwapChain& swap_chain) {
    return swap_chain.acquireNextImage(*image_available_semaphore);
}

void FrameSync::submitTo(vk::raii::Queue& graphics_queue) {
    const auto wait_dst_stage_mask = vk::PipelineStageFlags(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    auto submit_info = vk::SubmitInfo(*image_available_semaphore, wait_dst_stage_mask, *command_buffer, *render_finished_semaphore);

    graphics_queue.submit(submit_info, *in_flight_fence);
}

vk::Result FrameSync::presentTo(vk::raii::Queue& present_queue, SwapChain& swap_chain, uint32_t image_index) {
    auto present_info = vk::PresentInfoKHR(*render_finished_semaphore, swap_chain.get(), image_index);
    return present_queue.presentKHR(present_info);
}

vk::raii::CommandBuffer FrameSync::createCommandBuffer(const vk::raii::Device& device, const vk::CommandPool& command_pool) {
    auto allocate_info = vk::CommandBufferAllocateInfo(command_pool, vk::CommandBufferLevel::ePrimary, 1);
    return std::move(vk::raii::CommandBuffers(device, allocate_info).front());
}

}  // namespace visualization