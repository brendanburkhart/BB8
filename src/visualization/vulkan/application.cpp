#include "application.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>

#include "glm.hpp"
#include "shaders.hpp"
#include "shaders/uniform_buffer_object.hpp"
#include "shaders/vertex.hpp"

namespace visualization {
namespace vulkan {

const std::vector<std::string> Application::validation_layers = {"VK_LAYER_KHRONOS_validation"};
const std::vector<std::string> Application::device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

Application::Application(std::string name, Window* window)
    : window(window),
      instance(buildInstance(context, window, name, VK_MAKE_VERSION(0, 0, 1))),
      surface(window->createSurface(instance)),
      device(instance, *surface, validation_layers, device_extensions),
      descriptor_set_layout(buildDescriptorLayout(device)),
      pipeline_layout(nullptr),
      render_pass(nullptr),
      pipeline(nullptr),
      command_pool(device.createPool(false)),
      transient_pool(device.createPool(true)),
      descriptor_pool(createDescriptorPool(device)),
      texture_image(createTexture(device, *transient_pool)),
      depth_buffer(device, 1, 1),
      vertex_buffer(buildVertexBuffer()),
      index_buffer(buildIndexBuffer()),
      frames({FrameResources(device, *command_pool, *descriptor_pool, *descriptor_set_layout, texture_image),
              FrameResources(device, *command_pool, *descriptor_pool, *descriptor_set_layout, texture_image)}),
      swap_chain(device, *surface, window->size()) {
    buildRenderPass();
    buildSwapChain();
    buildGraphicsPipeline();
}

void Application::update() {
    drawFrame();
}

void Application::exit() {
    device.waitIdle();
}

void Application::onResize() {
    buildSwapChain();
}

vk::raii::DescriptorSetLayout Application::buildDescriptorLayout(const Device& device) {
    auto ubo_layout_binding = shaders::UniformBufferObject::layoutBinding();
    auto texture_sampler_layout_binding = vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment);
    std::vector<vk::DescriptorSetLayoutBinding> layout_bindings = {ubo_layout_binding, texture_sampler_layout_binding};

    auto descriptor_layout_create_info = vk::DescriptorSetLayoutCreateInfo({}, layout_bindings);
    return vk::raii::DescriptorSetLayout(device.logical(), descriptor_layout_create_info);
}

vk::raii::Instance Application::buildInstance(const vk::raii::Context& context, Window* window, std::string app_name, uint32_t app_version) {
    vk::ApplicationInfo app_info = vk::ApplicationInfo(app_name.c_str(), app_version, nullptr, 0, api_version);

    auto required_layers = enable_validation_layers ? validation_layers : std::vector<std::string>();
    auto enabled_layers = gatherLayers(context.enumerateInstanceLayerProperties(), required_layers);

    auto required_extensions = window->requiredVulkanExtensions();
    required_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    auto enabled_extensions = gatherExtensions(context.enumerateInstanceExtensionProperties(), required_extensions);

    auto create_info = vk::InstanceCreateInfo({}, &app_info, enabled_layers, enabled_extensions);
    auto instance = vk::raii::Instance(context, create_info);

    return instance;
}

Device Application::buildDevice(const vk::raii::Instance& instance, const vk::SurfaceKHR& surface) {
    auto required_layers = enable_validation_layers ? validation_layers : std::vector<std::string>();

    return Device(instance, surface, required_layers, device_extensions);
}

void Application::buildSwapChain() {
    device.waitIdle();

    swap_chain = SwapChain(device, *surface, window->size());
    depth_buffer = DepthBuffer(device, swap_chain.getExtent().width, swap_chain.getExtent().height);
    swap_chain.initializeFramebuffers(device, *render_pass, depth_buffer);
}

void Application::buildRenderPass() {
    auto color_attachment = vk::AttachmentDescription(
        {},
        swap_chain.getFormat(),
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::ePresentSrcKHR);

    auto depth_attachment = vk::AttachmentDescription(
        {},
        depth_buffer.getFormat(),
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eDontCare,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eDepthStencilAttachmentOptimal);

    auto color_reference = vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal);
    auto depth_reference = vk::AttachmentReference(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);

    auto subpass = vk::SubpassDescription({}, vk::PipelineBindPoint::eGraphics, {}, color_reference, {}, &depth_reference, {});

    auto subpass_dependency = vk::SubpassDependency(
        VK_SUBPASS_EXTERNAL,
        0u,
        vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
        vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
        {},
        vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite,
        {});

    auto attachments = std::vector<vk::AttachmentDescription>{color_attachment, depth_attachment};
    auto render_pass_create_info = vk::RenderPassCreateInfo({}, attachments, subpass, subpass_dependency, nullptr);

    render_pass = vk::raii::RenderPass(device.logical(), render_pass_create_info, nullptr);
}

vk::raii::DescriptorPool Application::createDescriptorPool(const Device& device) {
    auto ubo_size = vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, max_frames_in_flight);
    auto sampler_size = vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, max_frames_in_flight);
    auto pool_sizes = std::vector<vk::DescriptorPoolSize>{ubo_size, sampler_size};
    auto create_info = vk::DescriptorPoolCreateInfo(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, max_frames_in_flight, pool_sizes);

    return vk::raii::DescriptorPool(device.logical(), create_info);
}

Texture Application::createTexture(const Device& device, const vk::CommandPool& pool) {
    auto allocate_info = vk::CommandBufferAllocateInfo(pool, vk::CommandBufferLevel::ePrimary, 1);
    auto command_buffers = vk::raii::CommandBuffers(device.logical(), allocate_info);

    auto file = std::filesystem::path("resources/textures/statue.jpg");
    return Texture::load(device, file, vk::SamplerAddressMode::eRepeat, *command_buffers.front(), device.queue());
}

Buffer Application::buildVertexBuffer() {
    size_t size = vertex_data.size() * sizeof(vertex_data[0]);
    Buffer staging = Buffer(device, Buffer::Requirements::staging(size));
    Buffer vertex_buffer = Buffer(device, Buffer::Requirements::vertex(size));

    staging.fill((void*)vertex_data.data(), size);

    auto allocate_info = vk::CommandBufferAllocateInfo(*transient_pool, vk::CommandBufferLevel::ePrimary, 1);
    auto command_buffers = vk::raii::CommandBuffers(device.logical(), allocate_info);

    Buffer::copy(staging, vertex_buffer, *command_buffers.front(), device.queue());

    return vertex_buffer;
}

Buffer Application::buildIndexBuffer() {
    size_t size = vertex_indices.size() * sizeof(vertex_indices[0]);
    Buffer staging = Buffer(device, Buffer::Requirements::staging(size));
    Buffer index_buffer = Buffer(device, Buffer::Requirements::index(size));

    staging.fill((void*)vertex_indices.data(), size);

    auto allocate_info = vk::CommandBufferAllocateInfo(*transient_pool, vk::CommandBufferLevel::ePrimary, 1);
    auto command_buffers = vk::raii::CommandBuffers(device.logical(), allocate_info);

    Buffer::copy(staging, index_buffer, *command_buffers.front(), device.queue());

    return index_buffer;
}

void Application::buildGraphicsPipeline() {
    auto vert_shader_create_info = vk::ShaderModuleCreateInfo({}, shaders::vert_shader, nullptr);
    auto vert_shader_module = vk::raii::ShaderModule(device.logical(), vert_shader_create_info);

    auto frag_shader_create_info = vk::ShaderModuleCreateInfo({}, shaders::frag_shader, nullptr);
    auto frag_shader_module = vk::raii::ShaderModule(device.logical(), frag_shader_create_info);

    std::array<vk::PipelineShaderStageCreateInfo, 2> shader_stages = {
        vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eVertex, *vert_shader_module, "main"),
        vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eFragment, *frag_shader_module, "main")};

    auto binding_descriptions = shaders::Vertex::getBindingDescription();
    auto attribute_descriptions = shaders::Vertex::getAttributeDescriptions();
    auto vertex_input = vk::PipelineVertexInputStateCreateInfo(vk::PipelineVertexInputStateCreateFlags(), binding_descriptions, attribute_descriptions, nullptr);

    auto input_assembly = vk::PipelineInputAssemblyStateCreateInfo({}, vk::PrimitiveTopology::eTriangleList, false, nullptr);

    auto viewport_create_info = vk::PipelineViewportStateCreateInfo({}, 1, nullptr, 1, nullptr);

    std::vector<vk::DynamicState> dynamic_states = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
    auto dynamic_states_create_info = vk::PipelineDynamicStateCreateInfo({}, dynamic_states, nullptr);

    auto rasterizer = vk::PipelineRasterizationStateCreateInfo({},                                // flags
                                                               false,                             // depthClampEnable
                                                               false,                             // rasterizerDiscardEnable
                                                               vk::PolygonMode::eFill,            // polygonMode
                                                               vk::CullModeFlagBits::eBack,       // cullMode
                                                               vk::FrontFace::eCounterClockwise,  // frontFace
                                                               false,                             // depthBiasEnable
                                                               0.0f,                              // depthBiasConstantFactor
                                                               0.0f,                              // depthBiasClamp
                                                               0.0f,                              // depthBiasSlopeFactor
                                                               1.0f                               // lineWidth
    );

    auto multisample = vk::PipelineMultisampleStateCreateInfo({}, vk::SampleCountFlagBits::e1);

    using ccflags = vk::ColorComponentFlagBits;
    auto color_write_mask = vk::ColorComponentFlags(ccflags::eR | ccflags::eG | ccflags::eB | ccflags::eA);
    auto color_blend_attachment = vk::PipelineColorBlendAttachmentState(false,                   // blendEnable
                                                                        vk::BlendFactor::eZero,  // srcColorBlendFactor
                                                                        vk::BlendFactor::eZero,  // dstColorBlendFactor
                                                                        vk::BlendOp::eAdd,       // colorBlendOp
                                                                        vk::BlendFactor::eZero,  // srcAlphaBlendFactor
                                                                        vk::BlendFactor::eZero,  // dstAlphaBlendFactor
                                                                        vk::BlendOp::eAdd,       // alphaBlendOp
                                                                        color_write_mask         // colorWriteMask
    );

    auto color_blend = vk::PipelineColorBlendStateCreateInfo({},                         // flags
                                                             false,                      // logicOpEnable
                                                             vk::LogicOp::eNoOp,         // logicOp
                                                             color_blend_attachment,     // attachments
                                                             {{1.0f, 1.0f, 1.0f, 1.0f}}  // blendConstants
    );

    auto layout_create_info = vk::PipelineLayoutCreateInfo({}, *descriptor_set_layout, {});
    pipeline_layout = vk::raii::PipelineLayout(device.logical(), layout_create_info);

    auto depth_stencil = vk::PipelineDepthStencilStateCreateInfo(
        {},                    // flags
        true,                  // depth test enable
        true,                  // depth write enable
        vk::CompareOp::eLess,  // depth compare op
        false,                 // bounds test enable
        false,                 // stencil test enable
        {},                    // front
        {},                    // bacl
        0.0,                   // min depth bound
        1.0                    // max depth bound
    );

    auto pipeline_create_info = vk::GraphicsPipelineCreateInfo(
        {},                           // flags
        shader_stages,                // stages
        &vertex_input,                // vertex input state
        &input_assembly,              // input assembly state
        nullptr,                      // tessellation state
        &viewport_create_info,        // viewport state
        &rasterizer,                  // rasterization state
        &multisample,                 // multisample state
        &depth_stencil,                      // depth stencil state
        &color_blend,                 // color blend state
        &dynamic_states_create_info,  // dynamic state
        *pipeline_layout,             // layout
        *render_pass                  // render pass
    );

    pipeline = vk::raii::Pipeline(device.logical(), nullptr, pipeline_create_info);
}

void Application::updateUniformBuffer() {
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    shaders::UniformBufferObject ubo;
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    float aspect_ratio = swap_chain.getExtent().width / (float)swap_chain.getExtent().height;
    ubo.projection = glm::perspective(glm::radians(45.0f), aspect_ratio, 0.1f, 10.0f);
    ubo.projection[1][1] *= -1.0;

    frames[frame_index].writeUniformBuffer(ubo);
}

void Application::recordCommandBuffer(vk::CommandBuffer command_buffer, const vk::Framebuffer& framebuffer) {
    auto buffer_begin_info = vk::CommandBufferBeginInfo({}, nullptr);
    command_buffer.begin(buffer_begin_info);

    auto clear_color = vk::ClearValue(vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f));
    auto clear_depth = vk::ClearValue(vk::ClearDepthStencilValue(1.0, 0));
    auto clear_values = std::vector<vk::ClearValue>{clear_color, clear_depth};
    auto render_area = vk::Rect2D({0, 0}, swap_chain.getExtent());
    auto render_pass_info = vk::RenderPassBeginInfo(*render_pass, framebuffer, render_area, clear_values);
    command_buffer.beginRenderPass(render_pass_info, vk::SubpassContents::eInline);

    command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);
    command_buffer.bindVertexBuffers(0, vertex_buffer.get(), vk::DeviceSize(0));
    command_buffer.bindIndexBuffer(index_buffer.get(), vk::DeviceSize(0), vk::IndexType::eUint16);

    command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipeline_layout, 0, frames[frame_index].getDescriptors(), {});

    auto viewport = vk::Viewport(0.0, 0.0, swap_chain.getExtent().width, swap_chain.getExtent().height, 0.0, 1.0);
    command_buffer.setViewport(0, viewport);
    command_buffer.setScissor(0, render_area);

    command_buffer.drawIndexed(static_cast<uint32_t>(vertex_indices.size()), 1, 0, 0, 0);

    command_buffer.endRenderPass();
    command_buffer.end();
}

void Application::drawFrame() {
    auto& frame = frames[frame_index];

    frame.waitUntilReady(device);

    auto [acquire_result, image_index] = frame.acquireNextImage(swap_chain);
    if (acquire_result == vk::Result::eErrorOutOfDateKHR) {
        return;
    } else if (acquire_result != vk::Result::eSuccess && acquire_result != vk::Result::eSuboptimalKHR) {
        throw std::runtime_error("failed to acquire swap chain image");
    }
    assert(image_index < swap_chain.length());

    frame.reset(device);

    updateUniformBuffer();

    recordCommandBuffer(frame.getCommandBuffer(), swap_chain.getFramebuffer(image_index));

    frame.submitTo(device.queue());

    auto present_result = frame.presentTo(device.presentQueue(), swap_chain, image_index);
    if (present_result == vk::Result::eErrorOutOfDateKHR) {
        return;
    } else if (present_result != vk::Result::eSuccess && present_result != vk::Result::eSuboptimalKHR) {
        throw std::runtime_error("failed to present rendered image to swap chain");
    }

    frame_index = (frame_index + 1) % max_frames_in_flight;
}

}  // namespace vulkan
}  // namespace visualization
