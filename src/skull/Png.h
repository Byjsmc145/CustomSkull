#pragma once

#include <optional>
#include <string>
#include <string_view>

#include "skull/Types.h"

namespace custom_skull {

std::optional<SkinImage> decodePng(std::string_view bytes);
std::string              encodePng(SkinImage const& image);

} // namespace custom_skull
