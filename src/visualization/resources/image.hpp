#ifndef BB8_VISUALIZATION_RESOURCES_IMAGE_HPP
#define BB8_VISUALIZATION_RESOURCES_IMAGE_HPP

#include <filesystem>
#include <tuple>

namespace visualization {
namespace resources {

class Image {
public:
    static Image load(std::filesystem::path image_file);

    ~Image();

    const unsigned char* data() const;
    uint32_t width() const;
    uint32_t height() const;
    size_t size() const;

private:
    Image(unsigned char* pixel_data, int width, int height, int channels);

    unsigned char* pixel_data;
    const std::tuple<int, int, int> dimensions;
};

}  // namespace resources
}  // namespace visualization

#endif  // !BB8_VISUALIZATION_RESOURCES_IMAGE_HPP
