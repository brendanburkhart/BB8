#include "window.hpp"

#include <sstream>
#include <stdexcept>

void throwGLFWError() {
    const char* description;
    int code = glfwGetError(&description);

    std::stringstream error_string;
    error_string << "Error " << code << ": " << description << std::endl;

    throw std::runtime_error(error_string.str());
}

namespace visualization {

Window::Window(std::string name) {
    int ok = glfwInit();
    if (ok != GLFW_TRUE) {
        throwGLFWError();
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window_handle = glfwCreateWindow(default_width, default_height, name.c_str(), nullptr, nullptr);
    if (window_handle == nullptr) {
        throwGLFWError();
    }
}

Window::~Window() {
    glfwDestroyWindow(window_handle);
    glfwTerminate();
}

bool Window::update() {
    if (glfwWindowShouldClose(window_handle)) {
        return false;
    }

    glfwPollEvents();
    return true;
}

std::vector<std::string> Window::requiredVulkanExtensions() const {
    uint32_t glfw_extensions_count = 0;
    const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extensions_count);

    if (glfw_extensions == nullptr) {
        throwGLFWError();
    }

    std::vector<std::string> vulkan_extension_names;
    for (uint32_t i = 0; i < glfw_extensions_count; i++) {
        vulkan_extension_names.push_back(glfw_extensions[i]);
    }

    return vulkan_extension_names;
}

vk::Extent2D Window::size() const {
    int width, height;
    glfwGetFramebufferSize(window_handle, &width, &height);

    return vk::Extent2D(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
}

vk::raii::SurfaceKHR Window::createSurface(vk::raii::Instance& vulkan_instance) {
    VkSurfaceKHR surface_handle;
    glfwCreateWindowSurface(static_cast<VkInstance>(*vulkan_instance), window_handle, nullptr, &surface_handle);

    return vk::raii::SurfaceKHR(vulkan_instance, surface_handle);
}

}  // namespace visualization
