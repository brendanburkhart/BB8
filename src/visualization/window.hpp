#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <cstdint>
#include <string>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan_raii.hpp>

namespace visualization {

class Window {
public:
    Window(std::string name);
    ~Window();

    std::vector<std::string> requiredVulkanExtensions() const;
    vk::Extent2D size() const;

    vk::raii::SurfaceKHR createSurface(vk::raii::Instance& vulkan_instance);

    bool update();

private:
    static constexpr uint32_t default_width = 800;
    static constexpr uint32_t default_height = 600;

    GLFWwindow* window_handle;
};

}  // namespace visualization

#endif  // !WINDOW_HPP
