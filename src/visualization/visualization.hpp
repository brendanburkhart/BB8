#ifndef BB8_VISUALIZATION_VISUALIZATION_HPP
#define BB8_VISUALIZATION_VISUALIZATION_HPP

#include <string>

#include "vulkan_application.hpp"
#include "window.hpp"

namespace visualization {

class Visualization {
public:
    Visualization(std::string name);

    void run();

private:
    visualization::Window window;
    visualization::VulkanApplication vulkan;

    bool minimized;

    void resizeCallback();
};

}  // namespace visualization

#endif  // !BB8_VISUALIZATION_VISUALIZATION_HPP
