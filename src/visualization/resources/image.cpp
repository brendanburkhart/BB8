#include "image.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>

namespace visualization {
namespace resources {

Image::Image(unsigned char* pixel_data, int width, int height, int channels)
    : pixel_data(pixel_data), dimensions({width, height, channels}) {}

Image Image::load(std::filesystem::path image_file) {
    int width, height, source_channels;
    int desired_channels = STBI_rgb_alpha;
    stbi_uc* pixels = stbi_load(image_file.string().c_str(), &width, &height, &source_channels, desired_channels);

    if (!pixels) {
        throw std::runtime_error("failed to load texture image");
    }

    return Image(pixels, width, height, desired_channels);
}

Image::~Image() {
    stbi_image_free(pixel_data);
}

const unsigned char* Image::data() const {
    return pixel_data;
}

uint32_t Image::width() const {
    return static_cast<uint32_t>(std::get<0>(dimensions));
}
uint32_t Image::height() const {
    return static_cast<uint32_t>(std::get<1>(dimensions));
}
size_t Image::size() const {
    return static_cast<size_t>(std::get<0>(dimensions) * std::get<1>(dimensions) * std::get<2>(dimensions));
}

}  // namespace resources
}  // namespace visualization
