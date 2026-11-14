#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace custom_skull {

struct SkinImage {
    uint32_t             width  = 0;
    uint32_t             height = 0;
    bool                 slim   = false;
    std::vector<uint8_t> rgba;

    [[nodiscard]] bool valid() const {
        return width > 0 && height > 0 && rgba.size() == static_cast<size_t>(width) * height * 4;
    }
};

struct SkinRecord {
    std::string uuid;
    std::string name;
    std::string source;
    std::string file;
    bool        slim   = false;
    int         width  = 0;
    int         height = 0;
};

} // namespace custom_skull
