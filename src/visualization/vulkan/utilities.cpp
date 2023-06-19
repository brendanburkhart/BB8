#include "utilities.hpp"

#include <sstream>

namespace visualization {
namespace vulkan {

std::vector<const char*> gatherLayers(const std::vector<vk::LayerProperties> available_layers, const std::vector<std::string>& required_layers) {
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

std::vector<const char*> gatherExtensions(const std::vector<vk::ExtensionProperties> available_extensions, const std::vector<std::string>& required_extensions) {
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

}  // namespace vulkan
}  // namespace visualization
