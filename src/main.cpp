#include <iostream>

#include "visualization/visualization.hpp"

int main() {
    visualization::Visualization visualization("BB-8 Simulation");

    try {
        visualization.run();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
