#include "device.hpp"

#include <algorithm>
#include <set>
#include <unordered_set>
#include <vector>

#include "swap_chain.hpp"
#include "utilities.hpp"

namespace visualization {
namespace vulkan {

Device::Device(const vk::raii::Instance& instance, const vk::SurfaceKHR& surface, Layers layers, Extensions extensions)
    : physical_device(selectPhysicalDevice(instance, surface, layers, extensions)),
      queue_families(queryQueueFamilies(*physical_device, surface)),
      enabled_features(selectFeatures(*physical_device)),
      logical_device(buildLogicalDevice(physical_device, queue_families, enabled_features, layers, extensions)),
      graphics_queue(logical_device.getQueue(queue_families.graphics.value(), 0)),
      present_queue(logical_device.getQueue(queue_families.present.value(), 0)) {}

void Device::waitIdle() {
    logical_device.waitIdle();
}

vk::raii::CommandPool Device::createPool(bool transient) {
    auto flags = transient ? vk::CommandPoolCreateFlagBits::eTransient : vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    auto create_info = vk::CommandPoolCreateInfo(flags, queue_families.graphics.value());
    return vk::raii::CommandPool(logical_device, create_info);
}

const vk::PhysicalDevice Device::physical() const {
    return *physical_device;
}

const vk::raii::Device& Device::logical() const {
    return logical_device;
}

const vk::Queue& Device::queue() const {
    return *graphics_queue;
}

const vk::Queue& Device::presentQueue() const {
    return *present_queue;
}

const vk::PhysicalDeviceProperties Device::properties() const {
    return physical_device.getProperties();
}

const vk::PhysicalDeviceFeatures Device::features() const {
    return enabled_features;
}

Device::QueueFamilies Device::queryQueueFamilies(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface) {
    auto available_queue_families = device.getQueueFamilyProperties();

    std::unordered_set<uint32_t> graphics_families;
    std::unordered_set<uint32_t> present_families;

    for (uint32_t index = 0; index < available_queue_families.size(); index++) {
        if (available_queue_families[index].queueFlags & vk::QueueFlagBits::eGraphics) {
            graphics_families.insert(index);
        }

        vk::Bool32 present_support = device.getSurfaceSupportKHR(index, surface);
        if (present_support) {
            present_families.insert(index);
        }
    }

    // Ensure we choose same queue family for both graphics and presentation,
    // if a queue that supports both is available
    std::vector<uint32_t> intersection;
    std::set_intersection(graphics_families.begin(), graphics_families.end(), present_families.begin(), present_families.end(),
                          std::back_inserter(intersection));

    QueueFamilies families;
    if (intersection.size() > 0) {
        families.graphics = intersection.at(0);
        families.present = intersection.at(0);
    } else {
        families.graphics = graphics_families.size() > 0 ? *graphics_families.begin() : std::optional<uint32_t>();
        families.present = present_families.size() > 0 ? *present_families.begin() : std::optional<uint32_t>();
    }

    return families;
}

bool Device::supportsLayers(const vk::PhysicalDevice& device, const Layers layers) {
    try {
        gatherLayers(device.enumerateDeviceLayerProperties(), layers);
    } catch (std::runtime_error&) {
        return false;
    }

    return true;
}

bool Device::supportsExtensions(const vk::PhysicalDevice& device, const Extensions extensions) {
    try {
        gatherExtensions(device.enumerateDeviceExtensionProperties(), extensions);
    } catch (std::runtime_error&) {
        return false;
    }

    return true;
}

bool Device::supportsSwapChain(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface) {
    auto support = SwapChain::Support::query(device, surface);
    return !support.formats.empty() && !support.present_modes.empty();
}

vk::raii::PhysicalDevice Device::selectPhysicalDevice(const vk::raii::Instance& instance, const vk::SurfaceKHR& surface, const Layers required_layers, const Extensions required_extensions) {
    vk::raii::PhysicalDevices devices(instance);

    if (devices.size() == 0) {
        throw std::runtime_error("no Vulkan devices available");
    }

    std::vector<int> device_scores;
    for (auto&& device : devices) {
        bool layer_support = supportsLayers(*device, required_layers);
        bool extension_support = supportsExtensions(*device, required_extensions);

        if (!layer_support || !extension_support) {
            device_scores.push_back(0);
            continue;
        }

        if (!supportsSwapChain(*device, surface)) {
            device_scores.push_back(0);
            continue;
        }

        auto queue_families = queryQueueFamilies(*device, surface);
        if (!queue_families.graphics.has_value() || !queue_families.present.has_value()) {
            device_scores.push_back(0);
            continue;
        }

        int score = 1;
        auto supports_anisotropy = device.getFeatures().samplerAnisotropy;
        if (supports_anisotropy) {
            score += 1;
        }

        if (queue_families.graphics == queue_families.present) {
            score += 2;
        }

        device_scores.push_back(score);
    }

    auto max_score = std::max_element(std::begin(device_scores), std::end(device_scores));
    if (*max_score == 0) {
        throw std::runtime_error("no viable Vulkan devices are available");
    }

    auto device_index = std::distance(std::begin(device_scores), max_score);
    return devices.at(device_index);
}

vk::PhysicalDeviceFeatures Device::selectFeatures(const vk::PhysicalDevice& physical_device) {
    auto supports_anisotropy = physical_device.getFeatures().samplerAnisotropy;
    auto features = vk::PhysicalDeviceFeatures();
    features.samplerAnisotropy = supports_anisotropy;
    return features;
}

vk::raii::Device Device::buildLogicalDevice(const vk::raii::PhysicalDevice& physical_device, const QueueFamilies& queue_families, vk::PhysicalDeviceFeatures features, const Layers required_layers, const Extensions required_extensions) {
    constexpr float queue_priority = 0.0f;

    std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
    std::set<uint32_t> unique_queue_families = {queue_families.graphics.value(), queue_families.present.value()};
    for (uint32_t family : unique_queue_families) {
        vk::DeviceQueueCreateInfo queueCreateInfo({}, family, 1, &queue_priority);
        queue_create_infos.push_back(queueCreateInfo);
    }

    auto enabled_layers = gatherLayers(physical_device.enumerateDeviceLayerProperties(), required_layers);
    auto enabled_extensions = gatherExtensions(physical_device.enumerateDeviceExtensionProperties(), required_extensions);

    auto device_create_info = vk::DeviceCreateInfo(vk::DeviceCreateFlags(), queue_create_infos, enabled_layers, enabled_extensions, &features);
    return vk::raii::Device(physical_device, device_create_info);
}

}  // namespace vulkan
}  // namespace visualization
