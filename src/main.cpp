#include <iostream>

#include "visualization/window.hpp"
#include "visualization/vulkan_application.hpp"

int main() {
    visualization::Window window("BB-8 Simulation");
    visualization::VulkanApplication vulkan("BB-8 Simulation", &window);

    try {
        vulkan.run();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
