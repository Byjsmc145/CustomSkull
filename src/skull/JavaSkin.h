#pragma once

#include <optional>
#include <string>

#include "skull/Types.h"

namespace custom_skull {

struct JavaSkinResult {
    SkinImage   image;
    std::string name;
};

std::optional<JavaSkinResult> fetchJavaSkin(std::string const& playerName, std::string& error);

} // namespace custom_skull
