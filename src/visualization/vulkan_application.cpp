#include "vulkan_application.hpp"

#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>

#include "shaders.hpp"

namespace visualization {

const std::vector<std::string> VulkanApplication::validation_layers = {"VK_LAYER_KHRONOS_validation"};
const std::vector<std::string> VulkanApplication::device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

VulkanApplication::VulkanApplication(std::string name, Window* window)
    : window(window),
      instance(buildInstance(context, window, name, VK_MAKE_VERSION(0, 0, 1))),
      surface(window->createSurface(instance)),
      physical_device(selectPhysicalDevice(instance)),
      queue_family_indices(findQueueFamilies(physical_device, surface)),
      device(buildLogicalDevice(queue_family_indices, physical_device)),
      graphics_queue(device.getQueue(queue_family_indices.graphics_family.value(), 0)),
      present_queue(device.getQueue(queue_family_indices.present_family.value(), 0)),
      pipeline_layout(nullptr),
      render_pass(nullptr),
      pipeline(nullptr),
      command_pool(createCommandPool(device, queue_family_indices.graphics_family.value())),
      frame_sync({FrameSync(device, *command_pool), FrameSync(device, *command_pool)}),
      swap_chain(*physical_device, device, *surface, window->size()) {
    render_pass = buildRenderPass(device, swap_chain.getFormat());
    swap_chain.initializeFramebuffers(device, *render_pass);
    buildGraphicsPipeline();
}

void VulkanApplication::update() {
    drawFrame();
}

void VulkanApplication::exit() {
    device.waitIdle();
}

void VulkanApplication::onResize() {
    buildSwapChain();
}

std::vector<const char*> VulkanApplication::gatherLayers(const std::vector<vk::LayerProperties> available_layers, const std::vector<std::string>& required_layers) {
    std::vector<const char*> layers;

    for (auto const& layer : required_layers) {
        auto it = std::find_if(
            available_layers.begin(), available_layers.end(),
            [layer](vk::LayerProperties l) { return layer == l.layerName; });

        if (it == available_layers.end()) {
            std::stringstream error_message;
            error_message << "missing required layer: " << layer;
            throw std::runtime_error(error_message.str());
        } else {
            layers.push_back(layer.data());
        }
    }

    return layers;
}

std::vector<const char*> VulkanApplication::gatherExtensions(const std::vector<vk::ExtensionProperties> available_extensions, const std::vector<std::string>& required_extensions) {
    std::vector<const char*> extensions;

    for (auto const& extension : required_extensions) {
        auto it = std::find_if(
            available_extensions.begin(), available_extensions.end(),
            [extension](vk::ExtensionProperties e) { return extension == e.extensionName; });

        if (it == available_extensions.end()) {
            std::stringstream error_message;
            error_message << "missing required extension: " << extension;
            throw std::runtime_error(error_message.str());
        } else {
            extensions.push_back(extension.data());
        }
    }

    return extensions;
}

VulkanApplication::QueueFamilyIndices VulkanApplication::findQueueFamilies(const vk::raii::PhysicalDevice& device, const vk::raii::SurfaceKHR& surface) {
    QueueFamilyIndices indices;

    auto queue_families = device.getQueueFamilyProperties();

    for (uint32_t index = 0; index < queue_families.size(); index++) {
        if (queue_families[index].queueFlags & vk::QueueFlagBits::eGraphics) {
            indices.graphics_family = index;
        }

        vk::Bool32 present_support = device.getSurfaceSupportKHR(index, *surface);
        if (present_support) {
            indices.present_family = index;
        }
    }

    return indices;
}

vk::raii::PhysicalDevice VulkanApplication::selectPhysicalDevice(const vk::raii::Instance& instance) {
    vk::raii::PhysicalDevices devices(instance);

    return std::move(devices.front());
}

vk::raii::CommandPool VulkanApplication::createCommandPool(const vk::raii::Device& device, uint32_t queue_family_index) {
    auto create_info = vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, queue_family_index);
    return vk::raii::CommandPool(device, create_info);
}

vk::raii::CommandBuffer VulkanApplication::createCommandBuffer(const vk::raii::Device& device, const vk::raii::CommandPool& command_pool) {
    auto allocate_info = vk::CommandBufferAllocateInfo(*command_pool, vk::CommandBufferLevel::ePrimary, 1);
    return std::move(vk::raii::CommandBuffers(device, allocate_info).front());
}

vk::raii::Instance VulkanApplication::buildInstance(const vk::raii::Context& context, Window* window, std::string app_name, uint32_t app_version) {
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

vk::raii::Device VulkanApplication::buildLogicalDevice(const QueueFamilyIndices& queue_family_indices, const vk::raii::PhysicalDevice& physical_device) {
    if (!queue_family_indices.graphics_family || !queue_family_indices.present_family) {
        throw std::runtime_error("cannot find queues for both graphics and present");
    }

    auto required_layers = enable_validation_layers ? validation_layers : std::vector<std::string>();
    auto enabled_layers = gatherLayers(physical_device.enumerateDeviceLayerProperties(), required_layers);
    auto enabled_extensions = gatherExtensions(physical_device.enumerateDeviceExtensionProperties(), device_extensions);

    float queue_priority = 0.0f;

    vk::DeviceQueueCreateInfo queue_create_info(vk::DeviceQueueCreateFlags(), queue_family_indices.graphics_family.value(), 1, &queue_priority);
    vk::DeviceCreateInfo device_create_info(vk::DeviceCreateFlags(), queue_create_info, enabled_layers, enabled_extensions, nullptr);
    return vk::raii::Device(physical_device, device_create_info);
}

void VulkanApplication::buildSwapChain() {
    device.waitIdle();

    swap_chain = SwapChain(*physical_device, device, *surface, window->size());
    swap_chain.initializeFramebuffers(device, *render_pass);
}

vk::raii::RenderPass VulkanApplication::buildRenderPass(const vk::raii::Device& device, vk::Format color_format) {
    auto color_attachment = vk::AttachmentDescription(
        {},
        color_format,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::ePresentSrcKHR);

    auto color_reference = vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal);
    auto subpass = vk::SubpassDescription({}, vk::PipelineBindPoint::eGraphics, {}, color_reference, {}, {}, {});

    auto subpass_dependency = vk::SubpassDependency(
        VK_SUBPASS_EXTERNAL,
        0u,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        {},
        vk::AccessFlagBits::eColorAttachmentWrite,
        {});

    auto render_pass_create_info = vk::RenderPassCreateInfo({}, color_attachment, subpass, subpass_dependency, nullptr);
    return vk::raii::RenderPass(device, render_pass_create_info, nullptr);
}

void VulkanApplication::buildGraphicsPipeline() {
    auto vert_shader_create_info = vk::ShaderModuleCreateInfo({}, shaders::vert_shader, nullptr);
    auto vert_shader_module = vk::raii::ShaderModule(device, vert_shader_create_info);

    auto frag_shader_create_info = vk::ShaderModuleCreateInfo({}, shaders::frag_shader, nullptr);
    auto frag_shader_module = vk::raii::ShaderModule(device, frag_shader_create_info);

    std::array<vk::PipelineShaderStageCreateInfo, 2> shader_stages = {
        vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eVertex, *vert_shader_module, "main"),
        vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eFragment, *frag_shader_module, "main")};

    auto vertex_input = vk::PipelineVertexInputStateCreateInfo({}, {}, {}, nullptr);

    auto input_assembly = vk::PipelineInputAssemblyStateCreateInfo({}, vk::PrimitiveTopology::eTriangleList, false, nullptr);

    auto viewport_create_info = vk::PipelineViewportStateCreateInfo({}, 1, nullptr, 1, nullptr);

    std::vector<vk::DynamicState> dynamic_states = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
    auto dynamic_states_create_info = vk::PipelineDynamicStateCreateInfo({}, dynamic_states, nullptr);

    auto rasterizer = vk::PipelineRasterizationStateCreateInfo({},                           // flags
                                                               false,                        // depthClampEnable
                                                               false,                        // rasterizerDiscardEnable
                                                               vk::PolygonMode::eFill,       // polygonMode
                                                               vk::CullModeFlagBits::eBack,  // cullMode
                                                               vk::FrontFace::eClockwise,    // frontFace
                                                               false,                        // depthBiasEnable
                                                               0.0f,                         // depthBiasConstantFactor
                                                               0.0f,                         // depthBiasClamp
                                                               0.0f,                         // depthBiasSlopeFactor
                                                               1.0f                          // lineWidth
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

    auto layout_create_info = vk::PipelineLayoutCreateInfo({}, {}, {}, nullptr);
    pipeline_layout = vk::raii::PipelineLayout(device, layout_create_info);

    auto pipeline_create_info = vk::GraphicsPipelineCreateInfo(
        {},
        shader_stages,
        &vertex_input,
        &input_assembly,
        nullptr,
        &viewport_create_info,
        &rasterizer,
        &multisample,
        nullptr,
        &color_blend,
        &dynamic_states_create_info,
        *pipeline_layout,
        *render_pass);

    pipeline = vk::raii::Pipeline(device, nullptr, pipeline_create_info);
}

void VulkanApplication::recordCommandBuffer(vk::CommandBuffer command_buffer, const vk::Framebuffer& framebuffer) {
    auto buffer_begin_info = vk::CommandBufferBeginInfo({}, nullptr);
    command_buffer.begin(buffer_begin_info);

    auto clear_color = vk::ClearValue(vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f));
    auto render_area = vk::Rect2D({0, 0}, swap_chain.getExtent());
    auto render_pass_info = vk::RenderPassBeginInfo(*render_pass, framebuffer, render_area, clear_color);
    command_buffer.beginRenderPass(render_pass_info, vk::SubpassContents::eInline);

    command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);

    auto viewport = vk::Viewport(0.0, 0.0, swap_chain.getExtent().width, swap_chain.getExtent().height, 0.0, 1.0);
    command_buffer.setViewport(0, viewport);
    command_buffer.setScissor(0, render_area);

    command_buffer.draw(3, 1, 0, 0);

    command_buffer.endRenderPass();
    command_buffer.end();
}

void VulkanApplication::drawFrame() {
    frame_sync[frame_index].waitUntilReady(*device);

    auto [acquire_result, image_index] = frame_sync[frame_index].acquireNextImage(swap_chain);
    if (acquire_result == vk::Result::eErrorOutOfDateKHR) {
        return;
    } else if (acquire_result != vk::Result::eSuccess && acquire_result != vk::Result::eSuboptimalKHR) {
        throw std::runtime_error("failed to acquire swap chain image");
    }
    assert(image_index < swap_chain.length());

    frame_sync[frame_index].reset(*device);
    recordCommandBuffer(frame_sync[frame_index].getCommandBuffer(), swap_chain.getFramebuffer(image_index));

    frame_sync[frame_index].submitTo(graphics_queue);

    auto present_result = frame_sync[frame_index].presentTo(present_queue, swap_chain, image_index);
    if (present_result == vk::Result::eErrorOutOfDateKHR) {
        return;
    } else if (present_result != vk::Result::eSuccess && present_result != vk::Result::eSuboptimalKHR) {
        throw std::runtime_error("failed to present rendered image to swap chain");
    }

    frame_index = (frame_index + 1) % max_frames_in_flight;
}

}  // namespace visualization
