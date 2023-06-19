#include "frame_resources.hpp"

#include "shaders/uniform_buffer_object.hpp"

namespace visualization {

FrameResources::FrameResources(const vk::PhysicalDevice& physical_device, const vk::raii::Device& device,
                               const vk::CommandPool& command_pool,
                               const vk::DescriptorPool& descriptor_pool, const vk::DescriptorSetLayout& descriptor_layout)
    : command_buffer(createCommandBuffer(device, command_pool)),
      ubo_buffer(Buffer(physical_device, device, Buffer::Requirements::uniform(sizeof(shaders::UniformBufferObject)))),
      ubo_descriptor(createUBODescriptor(device, descriptor_pool, descriptor_layout, ubo_buffer)),
      image_available_semaphore(device, vk::SemaphoreCreateInfo()),
      render_finished_semaphore(device, vk::SemaphoreCreateInfo()),
      in_flight_fence(device, vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled)) {}

const vk::CommandBuffer& FrameResources::getCommandBuffer() const {
    return *command_buffer;
}

void FrameResources::waitUntilReady(const vk::Device& device) const {
    vk::Result wait_result = device.waitForFences(*in_flight_fence, true, timeout);
    assert(wait_result == vk::Result::eSuccess);
}

void FrameResources::reset(const vk::Device& device) {
    device.resetFences(*in_flight_fence);
    command_buffer.reset();
}

std::tuple<vk::Result, uint32_t> FrameResources::acquireNextImage(SwapChain& swap_chain) {
    return swap_chain.acquireNextImage(*image_available_semaphore);
}

void FrameResources::writeUniformBuffer(const shaders::UniformBufferObject& ubo) {
    std::memcpy(ubo_buffer.data(), &ubo, sizeof(ubo));
}

const vk::DescriptorSet& FrameResources::getDescriptor() const {
    return *ubo_descriptor;
}

void FrameResources::submitTo(vk::raii::Queue& graphics_queue) {
    const auto wait_dst_stage_mask = vk::PipelineStageFlags(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    auto submit_info = vk::SubmitInfo(*image_available_semaphore, wait_dst_stage_mask, *command_buffer, *render_finished_semaphore);

    graphics_queue.submit(submit_info, *in_flight_fence);
}

vk::Result FrameResources::presentTo(vk::raii::Queue& present_queue, SwapChain& swap_chain, uint32_t image_index) {
    auto present_info = vk::PresentInfoKHR(*render_finished_semaphore, swap_chain.get(), image_index);
    return present_queue.presentKHR(present_info);
}

vk::raii::CommandBuffer FrameResources::createCommandBuffer(const vk::raii::Device& device, const vk::CommandPool& command_pool) {
    auto allocate_info = vk::CommandBufferAllocateInfo(command_pool, vk::CommandBufferLevel::ePrimary, 1);
    return std::move(vk::raii::CommandBuffers(device, allocate_info).front());
}

vk::raii::DescriptorSet FrameResources::createUBODescriptor(const vk::raii::Device& device,
                                                            const vk::DescriptorPool& descriptor_pool,
                                                            const vk::DescriptorSetLayout& descriptor_layout,
                                                            Buffer& ubo_buffer) {
    auto allocate_info = vk::DescriptorSetAllocateInfo(descriptor_pool, descriptor_layout);
    auto descriptor_sets = vk::raii::DescriptorSets(device, allocate_info);
    vk::raii::DescriptorSet descriptor = std::move(descriptor_sets.at(0));

    auto buffer_info = vk::DescriptorBufferInfo(ubo_buffer.get(), 0, sizeof(shaders::UniformBufferObject));
    auto descriptor_write = vk::WriteDescriptorSet(*descriptor, 0, 0, vk::DescriptorType::eUniformBuffer, {}, buffer_info);
    device.updateDescriptorSets(descriptor_write, {});

    return descriptor;
}

}  // namespace visualization