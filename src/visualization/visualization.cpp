#include "visualization.hpp"

#include <functional>

namespace visualization {

Visualization::Visualization(std::string name) : window(name), vulkan(name, &window), minimized(window.is_minimized()) {
    window.setResizeCallback(std::bind(&Visualization::resizeCallback, this));
}

void Visualization::run() {
    while (true) {
        bool should_close = minimized ? window.wait() : window.update();
        if (should_close) {
            break;
        } else if (!minimized) {
            vulkan.update();
        }
    }

    vulkan.exit();
}

void Visualization::resizeCallback() {
    minimized = window.is_minimized();
    if (!minimized) {
        vulkan.onResize();
    }
}

}  // namespace visualization
