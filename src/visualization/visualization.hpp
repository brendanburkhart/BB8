#ifndef BB8_VISUALIZATION_VISUALIZATION_HPP
#define BB8_VISUALIZATION_VISUALIZATION_HPP

#include <string>

#include "vulkan/application.hpp"
#include "vulkan/window.hpp"

namespace visualization {

class Visualization {
public:
    Visualization(std::string name);

    void run();

private:
    vulkan::Window window;
    vulkan::Application vulkan;

    bool minimized;

    void resizeCallback();
};

}  // namespace visualization

#endif  // !BB8_VISUALIZATION_VISUALIZATION_HPP
