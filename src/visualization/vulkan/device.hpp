#ifndef BB8_VISUALIZATION_VULKAN_DEVICE_HPP
#define BB8_VISUALIZATION_VULKAN_DEVICE_HPP

#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include <vulkan/vulkan_raii.hpp>

namespace visualization {
namespace vulkan {

class Device {
public:
    using Layers = std::vector<std::string>;
    using Extensions = std::vector<std::string>;

    Device(const vk::raii::Instance& instance, const vk::raii::SurfaceKHR& surface, Layers required_layers, Extensions required_extensions);

    void waitIdle();

    vk::raii::CommandPool createPool(bool transient);

    const vk::PhysicalDevice physical() const;
    const vk::raii::Device& logical() const;
    const vk::Queue& queue() const;
    const vk::Queue& presentQueue() const;

private:
    class QueueFamilies {
    public:
        std::optional<uint32_t> graphics;
        std::optional<uint32_t> present;
    };

    static vk::raii::PhysicalDevice selectPhysicalDevice(const vk::raii::Instance& instance, const vk::raii::SurfaceKHR& surface);
    static QueueFamilies queryQueueFamilies(const vk::PhysicalDevice& device, const vk::raii::SurfaceKHR& surface);
    static vk::raii::Device buildLogicalDevice(const vk::raii::PhysicalDevice& physical_device, const QueueFamilies& queue_families, const Layers required_layers, const Extensions required_extensions);

    const vk::raii::PhysicalDevice physical_device;
    const QueueFamilies queue_families;
    const vk::raii::Device logical_device;

    const vk::raii::Queue graphics_queue;
    const vk::raii::Queue present_queue;
};

}  // namespace vulkan
}  // namespace visualization

#endif  // !BB8_VISUALIZATION_VULKAN_DEVICE_HPP
