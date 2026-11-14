#include "skull/Png.h"

#include <cstring>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_NO_STDIO
#include "stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_WRITE_NO_STDIO
#include "stb/stb_image_write.h"

namespace custom_skull {

std::optional<SkinImage> decodePng(std::string_view bytes) {
    if (bytes.empty()) {
        return std::nullopt;
    }
    int width  = 0;
    int height = 0;
    int channels = 0;
    auto* pixels = stbi_load_from_memory(
        reinterpret_cast<stbi_uc const*>(bytes.data()),
        static_cast<int>(bytes.size()),
        &width,
        &height,
        &channels,
        4
    );
    if (!pixels) {
        return std::nullopt;
    }
    if (width <= 0 || height <= 0 || width > 1024 || height > 1024) {
        stbi_image_free(pixels);
        return std::nullopt;
    }
    SkinImage image;
    image.width  = static_cast<uint32_t>(width);
    image.height = static_cast<uint32_t>(height);
    image.rgba.resize(static_cast<size_t>(width) * height * 4);
    std::memcpy(image.rgba.data(), pixels, image.rgba.size());
    stbi_image_free(pixels);
    return image;
}

std::string encodePng(SkinImage const& image) {
    if (!image.valid()) {
        return {};
    }
    std::string output;
    stbi_write_png_to_func(
        [](void* context, void* data, int size) {
            auto* out = static_cast<std::string*>(context);
            out->append(static_cast<char const*>(data), static_cast<size_t>(size));
        },
        &output,
        static_cast<int>(image.width),
        static_cast<int>(image.height),
        4,
        image.rgba.data(),
        static_cast<int>(image.width) * 4
    );
    return output;
}

} // namespace custom_skull
