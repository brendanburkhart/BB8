#ifndef BB8_VISUALIZATION_VULKAN_UTILITIES_HPP
#define BB8_VISUALIZATION_VULKAN_UTILITIES_HPP

#include <vector>
#include <vulkan/vulkan_raii.hpp>

namespace visualization {
namespace vulkan {

std::vector<const char*> gatherLayers(const std::vector<vk::LayerProperties> available_layers, const std::vector<std::string>& required_layers);
std::vector<const char*> gatherExtensions(const std::vector<vk::ExtensionProperties> available_extensions, const std::vector<std::string>& required_extensions);

}  // namespace vulkan
}  // namespace visualization

#endif  // !BB8_VISUALIZATION_VULKAN_UTILITIES_HPP
