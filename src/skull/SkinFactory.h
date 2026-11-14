#pragma once

#include <string>
#include <string_view>

#include "mc/world/actor/player/SerializedSkinRef.h"
#include "skull/Types.h"

namespace custom_skull {

bool buildSerializedSkin(SerializedSkinRef& skin, SkinImage const& image, std::string const& skinId);

bool buildSkullEntitySkin(SerializedSkinRef& skin, SkinImage const& image, std::string const& skinId);

std::string_view skullEntityGeometry();

} // namespace custom_skull
