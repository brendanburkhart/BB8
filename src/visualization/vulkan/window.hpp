#ifndef BB8_VISUALIZATION_VULKAN_WINDOW_HPP
#define BB8_VISUALIZATION_VULKAN_WINDOW_HPP

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan_raii.hpp>

namespace visualization {
namespace vulkan {

class Window {
public:
    Window(std::string name);
    ~Window();

    std::vector<std::string> requiredVulkanExtensions() const;
    vk::Extent2D size() const;
    bool is_minimized() const;

    vk::raii::SurfaceKHR createSurface(vk::raii::Instance& vulkan_instance);

    bool update();
    bool wait();

    void setResizeCallback(std::function<void()> callback);

private:
    static constexpr uint32_t default_width = 800;
    static constexpr uint32_t default_height = 600;

    GLFWwindow* window_handle;

    std::function<void()> resize_callback;

    static void resizeCallback(GLFWwindow* window, int, int);
};

}  // namespace vulkan
}  // namespace visualization

#endif  // !BB8_VISUALIZATION_VULKAN_WINDOW_HPP
